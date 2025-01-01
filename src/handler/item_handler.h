#ifndef ITEM_HANDLER_H
#define ITEM_HANDLER_H

#include <string>
#include <optional>
#include "mysql_connector.h"
#include "handler.h"
#include "item.h"

using namespace std;

struct Item;
class ItemModel;

struct CreateItemDto
{
    string itemName;
    optional<double> buyNowPrice;
    string startTime;
    string endTime;
    int ownerId;
};

struct SearchItemDto
{
    optional<int> userId;
    optional<string> itemName;
    optional<string> startTime;
    optional<string> endTime;
    optional<int> roomId;
    optional<vector<string>> states;
};

struct UpdateItemDto
{
    optional<string> startTime;
    optional<string> endTime;
    optional<double> buyNowPrice;
    int userId;
    int itemId;
};

class ItemHandler : public Handler
{
public:
    static ItemHandler &getInstance(int connfd, string client_ip, int client_port, MySQLOperations *mysqlOps)
    {
        static ItemHandler instance(connfd, client_ip, client_port, mysqlOps);
        return instance;
    }

    void viewAllItems();
    void viewItemsInRoom(char *payload);
    void searchItem(char *payload);
    void createItem(char *payload);
    void sendItemList(vector<Item> items);
    void viewOwnedItems(char *payload);
    void updateItem(char *buf);
    void deleteItem(char *buf);
    ItemModel &itemModel;

private:
    ItemHandler(int connfd, string client_ip, int client_port, MySQLOperations *mysqlOps);
    ~ItemHandler() {}
    ItemHandler(const ItemHandler &) = delete;
    ItemHandler &operator=(const ItemHandler &) = delete;

    MySQLOperations *mysqlOps;
};

#endif // ITEM_HANDLER_H