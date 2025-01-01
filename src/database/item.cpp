#include <iostream>
#include <string>
#include <vector>
#include "mysql_connector.h"
#include "item.h"
#include <string.h>
#include "item_handler.h"

using namespace std;

vector<Item> ItemModel::getItems(optional<string> condition)
{
    vector<Item> items;
    string sql = "SELECT i.item_id, i.name, i.start_time, i.end_time, i.current_price, i.state, i.buy_now_price, i.owner_id, i.room_id, u.name AS owner_name "
                 "FROM item i "
                 "JOIN user u ON i.owner_id = u.user_id "
                 "WHERE 1=1";

    if (condition.has_value() && !condition.value().empty())
    {
        sql += condition.value();
    }
    sql += ";";
    cout << "SQL query: " << sql << '\n';

    sql::ResultSet *res = mysqlOps->stmt->executeQuery(sql);
    if (!res)
    {
        cout << "[-]Failed to retrieve items." << endl;
        return items;
    }

    while (res->next())
    {
        Item item = parseResultSet(res);
        if (item.roomId.has_value())
        {
            string roomSql = "SELECT name FROM room WHERE room_id = " + to_string(item.roomId.value()) + ";";
            cout << "SQL query: " << roomSql << '\n';
            sql::ResultSet *roomRes = mysqlOps->stmt->executeQuery(roomSql);
            if (roomRes && roomRes->next())
            {
                item.roomName = roomRes->getString("name");
            }
            delete roomRes;
        }
        else
        {
            item.roomName = std::nullopt;
        }
        items.push_back(item);
    }
    delete res;

    return items;
}

int ItemModel::createItem(const CreateItemDto &request)
{
    // check if the item already exists
    string checkSql = "SELECT 1 FROM item WHERE name = '" + request.itemName + "';";
    cout << "SQL query: " << checkSql << '\n';
    sql::ResultSet *checkRes = mysqlOps->stmt->executeQuery(checkSql);
    if (checkRes && checkRes->next())
    {
        cout << "[-]Item already existed." << endl;
        delete checkRes;
        return 2; // Already existed
    }
    delete checkRes;

    string sql;
    if (request.buyNowPrice.has_value())
    {
        sql = "INSERT INTO item(name, buy_now_price, owner_id, start_time, end_time) VALUES ('" + request.itemName + "'," + to_string(request.buyNowPrice.value()) + "," + to_string(request.ownerId) + ",'" + request.startTime + "','" + request.endTime + "');";
    }
    else
    {
        sql = "INSERT INTO item(name, owner_id, start_time, end_time) VALUES ('" + request.itemName + "'," + to_string(request.ownerId) + ",'" + request.startTime + "','" + request.endTime + "');";
    }

    cout << "SQL query: " << sql << '\n';
    bool res = mysqlOps->stmt->executeUpdate(sql);
    if (res)
        return 1; // SUCCESS
    else
        return 0; // FAIL
}

vector<Item> ItemModel::searchItem(const SearchItemDto &searchDto)
{
    string condition;

    if (searchDto.itemName.has_value())
    {
        condition += " AND i.name LIKE '%" + searchDto.itemName.value() + "%'";
    }

    if (searchDto.startTime.has_value())
    {
        condition += " AND i.start_time >= '" + searchDto.startTime.value() + "'";
    }

    if (searchDto.endTime.has_value())
    {
        condition += " AND i.end_time <= '" + searchDto.endTime.value() + "'";
    }

    if (searchDto.roomId.has_value())
    {
        condition += " AND i.room_id = " + to_string(searchDto.roomId.value());
    }

    if (searchDto.userId.has_value())
    {
        condition += " AND i.owner_id = " + to_string(searchDto.userId.value());
    }

    if (searchDto.states.has_value())
    {
        string states = "(";
        for (string state : searchDto.states.value())
        {
            states += "'" + state + "',";
        }
        states.pop_back();
        states += ")";
        condition += " AND i.state IN " + states;
    }

    return getItems(condition);
}

int ItemModel::updateItem(const UpdateItemDto &request)
{
    string sql = "UPDATE item SET ";

    if (request.startTime.has_value())
    {
        sql += "start_time = '" + request.startTime.value() + "' ,";
    }

    if (request.endTime.has_value())
    {
        sql += "end_time = '" + request.endTime.value() + "' ,";
    }

    if (request.buyNowPrice.has_value())
    {
        sql += "buy_now_price = " + to_string(request.buyNowPrice.value()) + " ,";
    }

    sql.pop_back();

    sql += "WHERE 1=1 AND item_id = " + to_string(request.itemId) + " AND owner_id = " + to_string(request.userId) + " AND state = 'created';";

    cout << "SQL query: " << sql << '\n';
    bool res = mysqlOps->stmt->executeUpdate(sql);
    if (res)
        return 1; // SUCCESS
    else
        return 0; // FAIL
}

int ItemModel::deleteItem(int itemId, int userId)
{
    string sql = "DELETE FROM item WHERE item_id = " + to_string(itemId) + " AND owner_id = " + to_string(userId) + " AND state = 'created';";
    cout << "SQL query: " << sql << '\n';
    bool res = mysqlOps->stmt->executeUpdate(sql);
    if (res)
        return 1; // SUCCESS
    else
        return 0; // FAIL
}

Item ItemModel::parseResultSet(sql::ResultSet *res)
{
    Item item;
    item.itemId = res->getInt("item_id");
    item.name = res->getString("name");
    item.startTime = res->getString("start_time");
    item.endTime = res->getString("end_time");
    item.currentPrice = res->getDouble("current_price");
    item.state = res->getString("state");
    item.buyNowPrice = res->getDouble("buy_now_price");
    item.ownerId = res->getInt("owner_id");
    if (res->isNull("room_id"))
    {
        item.roomId = std::nullopt;
    }
    else
    {
        item.roomId = res->getInt("room_id");
    }
    item.ownerName = res->getString("owner_name");
    return item;
}