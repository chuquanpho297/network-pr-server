#include <iostream>
#include <string>
#include <vector>
#include "mysql_connector.h"
#include "room.h"
#include <config.h>
#include <string.h>
#include "optional"

using namespace std;

int RoomModel::createRoom(const CreateRoomDto &dto)
{
    // Check if the room already exists
    string checkSql = "SELECT 1 FROM room WHERE name = '" + dto.roomName + "';";
    cout << "SQL query: " << checkSql << '\n';
    sql::ResultSet *checkRes = mysqlOps->stmt->executeQuery(checkSql);

    if (checkRes && checkRes->next())
    {
        cout << "[-]Room already existed." << endl;
        delete checkRes;
        return 2; // Already existed
    }
    delete checkRes;

    string sql = "INSERT INTO room(name, owner_id, start_time, end_time) VALUES ('" + dto.roomName + "'," + to_string(dto.userId) + ",'" + dto.startTime + "','" + dto.endTime + "');";
    bool res = mysqlOps->stmt->executeUpdate(sql);
    cout << "SQL query: " << sql << '\n';
    if (res)
        return 1; // SUCCESS
    else
        return 0; // FAIL
}

vector<Room> RoomModel::getRooms(optional<int> userId, optional<string> condition)
{
    vector<Room> rooms;
    string sql = "SELECT r.room_id, r.name, r.owner_id, u.name AS owner_name, r.start_time, r.end_time, "
                 "(SELECT COUNT(*) FROM item i WHERE i.room_id = r.room_id) AS totalItems, "
                 "(SELECT COUNT(*) FROM user u WHERE u.room_id = r.room_id) AS totalParticipants "
                 "FROM room r "
                 "JOIN user u ON r.owner_id = u.user_id "
                 "WHERE 1=1";

    if (condition.has_value() && !condition.value().empty())
    {
        sql += condition.value();
    }

    if (userId.has_value())
    {
        sql += " AND r.owner_id = " + to_string(userId.value());
    }
    sql += ";";

    cout << "SQL query: " << sql << '\n';

    sql::ResultSet *res = mysqlOps->stmt->executeQuery(sql);
    if (!res)
    {
        cout << "[-]Failed to retrieve rooms." << endl;
        return rooms;
    }

    while (res->next())
    {
        rooms.push_back(parseResultSet(res));
    }
    delete res;
    return rooms;
}

int RoomModel::joinRoom(int userId, int roomId)
{
    // Check if the user is not already in a room
    string checkSql = "SELECT room_id FROM user WHERE user_id = " + to_string(userId) + " AND room_id IS NULL;";
    cout << "SQL query: " << checkSql << '\n';
    sql::ResultSet *checkRes = mysqlOps->stmt->executeQuery(checkSql);

    if (checkRes && checkRes->next())
    {
        delete checkRes;
        // Update the user's room_id
        string updateSql = "UPDATE user SET room_id = " + to_string(roomId) + " WHERE user_id = " + to_string(userId) + ";";
        bool res = mysqlOps->stmt->executeUpdate(updateSql);
        cout << "SQL query: " << updateSql << '\n';
        if (res)
            return 1; // SUCCESS
        else
            return 0; // FAIL
    }
    else
    {
        delete checkRes;
        return 2; // Already in a room
    }
}

int RoomModel::placeBid(int userId, int itemId, double bidPrice)
{
    string query = "SELECT current_price FROM item WHERE item_id = " + to_string(itemId) + ";";
    cout << "SQL query: " << query << '\n';
    sql::ResultSet *result = mysqlOps->stmt->executeQuery(query);
    if (!result->next())
    {
        delete result;
        return 0; // Fail
    }

    double currentPrice = result->getDouble("current_price");
    delete result;
    if (bidPrice < currentPrice + 10000)
    {
        return 3; // Invalid bid price
    }

    string updateQuery = "UPDATE item SET current_price = " + to_string(bidPrice) + " WHERE item_id = " + to_string(itemId) + ";";
    cout << "SQL query: " << updateQuery << '\n';
    bool updateResult = mysqlOps->stmt->executeUpdate(updateQuery);
    if (updateResult > 0)
    {
        string logQuery = "INSERT INTO log (user_id, item_id, room_id, bid_price, time, status) VALUES (" +
                          to_string(userId) + ", " + to_string(itemId) +
                          ", (SELECT room_id FROM item WHERE item_id = " + to_string(itemId) + "), " +
                          to_string(bidPrice) + ", NOW(), 'success');";
        cout << "SQL query: " << logQuery << '\n';
        mysqlOps->stmt->executeUpdate(logQuery);
        return 1; // SUCCESS
    }
    else
    {
        return 0; // Fail
    }
}

int RoomModel::buyNow(int userId, int itemId)
{
    // get buynow price and state
    string query = "SELECT buy_now_price, state FROM item WHERE item_id = " + to_string(itemId) + ";";
    cout << "SQL query: " << query << '\n';
    sql::ResultSet *res = mysqlOps->stmt->executeQuery(query);

    if (!res->next())
    {
        return 0;
    }

    double buy_now_price = res->getDouble("buy_now_price");
    string state = res->getString("state");

    if (state != "active" || buy_now_price <= 0)
    {
        return 2;
    }

    string updateQuery = "UPDATE item SET state = 'sold', current_price = " + to_string(buy_now_price) +
                         " WHERE item_id = " + to_string(itemId) + ";";
    cout << "SQL query: " << updateQuery << '\n';
    bool updateSuccess = mysqlOps->stmt->executeUpdate(updateQuery);

    if (updateSuccess == 0)
    {
        return 0;
    }
    // add log into log table
    string logQuery = "INSERT INTO log(user_id, item_id, room_id, bid_price, time, status) VALUES (" +
                      to_string(userId) + ", " + to_string(itemId) + ", (SELECT room_id FROM item WHERE item_id = " + to_string(itemId) + "), " +
                      to_string(buy_now_price) + ", NOW(), 'success');";
    cout << "SQL query: " << logQuery << '\n';
    bool logSuccess = mysqlOps->stmt->executeUpdate(logQuery);

    if (logSuccess == 0)
    {
        return 0;
    }

    return 1;
}

int RoomModel::leaveRoom(int userId, int roomId)
{
    string updateQuery = "UPDATE user SET room_id = NULL WHERE user_id = " + to_string(userId) + " AND room_id = " + to_string(roomId) + ";";
    cout << "SQL query: " << updateQuery << '\n';
    bool res = mysqlOps->stmt->executeUpdate(updateQuery);
    return res ? 1 : 0;
}

int RoomModel::placeItemInRoom(int userId, int itemId, int roomId)
{
    string checkSql = "SELECT 1 FROM item WHERE item_id = " + to_string(itemId) + " AND owner_id = " + to_string(userId) + " AND state = 'created';";
    cout << "SQL query: " << checkSql << '\n';
    sql::ResultSet *checkRes = mysqlOps->stmt->executeQuery(checkSql);
    if (!checkRes || !checkRes->next())
    {
        cout << "[-]Item does not exist or is not owned by the user." << endl;
        delete checkRes;
        return 0; // Fail
    }
    delete checkRes;
    string insertQuery = "INSERT INTO room_log (item_id, room_id, status) VALUES (" +
                         to_string(itemId) + ", " + to_string(roomId) + ", 'pending');";
    int success = mysqlOps->stmt->executeUpdate(insertQuery);
    cout << "SQL query: " << insertQuery << '\n';
    return success ? 1 : 0;
}

int RoomModel::acceptRejectItem(int itemId, int roomId, int status)
{
    string checkQuery = "SELECT 1 FROM room_log WHERE item_id = " + to_string(itemId) +
                        " AND room_id = " + to_string(roomId) + " AND status = 'pending';";
    cout << "SQL query: " << checkQuery << '\n';
    sql::ResultSet *checkRes = mysqlOps->stmt->executeQuery(checkQuery);
    if (!checkRes || !checkRes->next())
    {
        cout << "[-]Item is not pending." << endl;
        delete checkRes;
        return 0; // Fail
    }

    string newStatus = (status == 1) ? "accepted" : "rejected";
    string updateQuery = "UPDATE room_log SET status = '" + newStatus + "' WHERE item_id = " +
                         to_string(itemId) + " AND room_id = " + to_string(roomId) + ";";
    cout << "SQL query: " << updateQuery << '\n';
    bool success = mysqlOps->stmt->executeUpdate(updateQuery) > 0;
    return success ? 1 : 0;
}

vector<RoomLog> RoomModel::getRoomLog(int roomId)
{
    vector<RoomLog> roomLogs;
    string sql = "SELECT rl.room_log_id, rl.item_id, rl.room_id, rl.status, rl.time, i.buy_now_price, i.name AS item_name "
                 "FROM room_log rl "
                 "JOIN item i ON rl.item_id = i.item_id "
                 "WHERE rl.room_id = " +
                 to_string(roomId) + ";";
    cout << "SQL query: " << sql << '\n';
    sql::ResultSet *res = mysqlOps->stmt->executeQuery(sql);
    if (!res)
    {
        cout << "[-]Failed to retrieve room logs." << endl;
        return roomLogs;
    }

    while (res->next())
    {
        roomLogs.push_back(parseRoomLogResultSet(res));
    }
    delete res;
    return roomLogs;
}

vector<UserLog> RoomModel::getUserLog(int roomId, int itemId)
{
    vector<UserLog> userLogs;
    string sql = "SELECT l.log_id, l.user_id, u.name AS user_name, l.item_id, l.room_id, l.bid_price, l.time, l.status "
                 "FROM log l "
                 "JOIN user u ON l.user_id = u.user_id "
                 "WHERE l.room_id = " +
                 to_string(roomId) + " AND l.item_id = " + to_string(itemId) + ";";
    cout << "SQL query: " << sql << '\n';
    sql::ResultSet *res = mysqlOps->stmt->executeQuery(sql);
    if (!res)
    {
        cout << "[-]Failed to retrieve user logs." << endl;
        return userLogs;
    }

    while (res->next())
    {
        UserLog userLog;
        userLog.logId = res->getInt("log_id");
        userLog.userId = res->getInt("user_id");
        userLog.userName = res->getString("user_name");
        userLog.itemId = res->getInt("item_id");
        userLog.roomId = res->getInt("room_id");
        userLog.bidPrice = res->getDouble("bid_price");
        userLog.time = res->getString("time");
        userLog.status = res->getString("status");
        userLogs.push_back(userLog);
    }
    delete res;
    return userLogs;
}

int RoomModel::deleteItemFromRoom(int userId, int itemId, int roomId)
{
    // check if the user is the owner of the item or owner of the room
    string checkSql = "SELECT 1 FROM item WHERE item_id = " + to_string(itemId) + " AND owner_id = " + to_string(userId) + ";";
    cout << "SQL query: " << checkSql << '\n';
    sql::ResultSet *checkRes = mysqlOps->stmt->executeQuery(checkSql);
    if (!checkRes || !checkRes->next())
    {
        cout << "[-]Item does not exist or is not owned by the user." << endl;
        delete checkRes;
        return 0; // Fail
    }
    delete checkRes;

    checkSql = "SELECT 1 FROM room WHERE room_id = " + to_string(roomId) + " AND owner_id = " + to_string(userId) + ";";
    if (!checkRes || !checkRes->next())
    {
        cout << "[-]Room does not exist or is not owned by the user." << endl;
        delete checkRes;
        return 0; // Fail
    }

    string updateQuery = "UPDATE item SET state = 'created' AND room_id = NULL WHERE item_id = " + to_string(itemId) + " AND room_id = " + to_string(roomId) + ";";
    cout << "SQL query: " << updateQuery << '\n';
    bool res = mysqlOps->stmt->executeUpdate(updateQuery);
    return res ? 1 : 0;
}

Room RoomModel::parseResultSet(sql::ResultSet *res)
{
    Room room;
    room.roomId = res->getInt("room_id");
    room.name = res->getString("name");
    room.ownerId = res->getInt("owner_id");
    room.ownerName = res->getString("owner_name");
    room.totalItems = res->getInt("totalItems");
    room.totalParticipants = res->getInt("totalParticipants");
    room.startTime = res->getString("start_time");
    room.endTime = res->getString("end_time");

    return room;
}

RoomLog RoomModel::parseRoomLogResultSet(sql::ResultSet *res)
{
    RoomLog roomLog;
    roomLog.roomLogId = res->getInt("room_log_id");
    roomLog.itemId = res->getInt("item_id");
    roomLog.roomId = res->getInt("room_id");
    roomLog.itemName = res->getString("item_name");
    roomLog.status = res->getString("status");
    roomLog.buyNowPrice = res->getInt("buy_now_price");
    roomLog.time = res->getString("time");

    return roomLog;
}