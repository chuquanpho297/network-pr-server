#ifndef ITEM_H
#define ITEM_H

#include <string>
#include <vector>
#include "mysql_connector.h"
#include <item_handler.h>

using namespace std;

struct CreateItemDto;
struct SearchItemDto;
struct UpdateItemDto;

struct Item
{
    int itemId;
    string name;
    string startTime;
    string endTime;
    double currentPrice;
    string state;
    double buyNowPrice;
    int ownerId;
    optional<int> roomId;
    optional<string> roomName;
    string ownerName;
};

class ItemModel
{
public:
    static ItemModel &getInstance(MySQLOperations *mysqlOps)
    {
        static ItemModel instance(mysqlOps);
        return instance;
    }

    vector<Item> getItems(optional<string> condition);
    int createItem(const CreateItemDto &request);
    vector<Item> searchItem(const SearchItemDto &request);
    int updateItem(const UpdateItemDto &request);
    int deleteItem(int itemId, int userId);

private:
    ItemModel(MySQLOperations *mysqlOps) : mysqlOps(mysqlOps) {}
    ~ItemModel() {}
    ItemModel(const ItemModel &) = delete;
    ItemModel &operator=(const ItemModel &) = delete;
    Item parseResultSet(sql::ResultSet *res);

    MySQLOperations *mysqlOps;
};

#endif // ITEM_H