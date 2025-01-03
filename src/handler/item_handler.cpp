#include "item_handler.h"
#include <item.h>
#include <config.h>
#include <string.h>
#include <util.h>

ItemHandler::ItemHandler(int connfd, string client_ip, int client_port, MySQLOperations *mysqlOps)
    : Handler(connfd, client_ip, client_port), mysqlOps(mysqlOps), itemModel(ItemModel::getInstance(mysqlOps)) {}

void ItemHandler::viewAllItems()
{
    char response[50];
    try
    {
        vector<Item> items = itemModel.getItems(optional<string>(" AND i.state IN ('waiting', 'active')"));
        sprintf(response, "%d\n%d\n", VIEW_ALL_ITEMS_RES, SUCCESS);
        log_send_msg(response);
        sendItemList(items);
    }
    catch (const exception &e)
    {
        printf("Exception caught in viewAllItems: %s\n", e.what());
        sprintf(response, "%d\n%d\n", VIEW_ALL_ITEMS_RES, FAIL);
        log_send_msg(response);
    }
}

void ItemHandler::viewItemsInRoom(char *payload)
{
    char response[50];
    int roomId;
    try
    {
        int noargs = sscanf(payload, "%d\n", &roomId);
        if (noargs == 1)
        {
            vector<Item> items = itemModel.getItems(optional<string>(" AND i.room_id = " + to_string(roomId)));
            sprintf(response, "%d\n%d\n", VIEW_ITEMS_IN_ROOM_RES, SUCCESS);
            log_send_msg(response);
            sendItemList(items);
        }
        else
        {
            printf("[-]Invalid view items in room protocol! %s\n", payload);
            sprintf(response, "%d\n%d\n", VIEW_ITEMS_IN_ROOM_RES, FAIL);
            log_send_msg(response);
        }
    }
    catch (const exception &e)
    {
        printf("Exception caught in viewItemsInRoom: %s\n", e.what());
        sprintf(response, "%d\n%d\n", VIEW_ITEMS_IN_ROOM_RES, FAIL);
        log_send_msg(response);
    }
}

void ItemHandler::viewOwnedItems(char *payload)
{
    char response[50];
    int userId;
    int checkIsPlaced;
    try
    {
        int noargs = sscanf(payload, "%d\n%d\n", &userId, &checkIsPlaced);
        if (noargs == 2)
        {
            string query = " AND i.owner_id = " + to_string(userId);
            if (checkIsPlaced == 1)
            {
                query += " AND (SELECT COUNT(*) FROM room_log rl WHERE rl.item_id = i.item_id) = 0";
            }
            vector<Item> items = itemModel.getItems(optional<string>(query));
            sprintf(response, "%d\n%d\n", VIEW_OWNED_ITEMS_RES, SUCCESS);
            log_send_msg(response);
            sendItemList(items);
        }
        else
        {
            printf("[-]Invalid view owned items protocol! %s\n", payload);
            sprintf(response, "%d\n%d\n", VIEW_OWNED_ITEMS_RES, FAIL);
            log_send_msg(response);
        }
    }
    catch (const exception &e)
    {
        printf("Exception caught in viewOwnedItems: %s\n", e.what());
        sprintf(response, "%d\n%d\n", VIEW_OWNED_ITEMS_RES, FAIL);
        log_send_msg(response);
    }
}

void ItemHandler::searchItem(char *payload)
{
    char itemName[MAX_ROOM_NAME], startTime[20], endTime[20], response[50], roomId[5], userId[5], states[100];
    try
    {
        int noargs = sscanf(payload, "%s\n%s\n%s\n%s\n%s\n%s\n", itemName, startTime, endTime, roomId, userId, states);
        if (noargs == 6)
        {
            optional<int> userIdOpt;
            optional<string> itemNameOpt;
            optional<string> startTimeOpt;
            optional<string> endTimeOpt;
            optional<int> roomIdOpt;
            optional<vector<string>> statesOpt;

            if (strcmp(itemName, NULL_TAG) != 0)
            {
                itemNameOpt = optional<string>(itemName);
            }

            if (strcmp(startTime, NULL_TAG) != 0)
            {
                startTimeOpt = optional<string>(startTime);
            }

            if (strcmp(endTime, NULL_TAG) != 0)
            {
                endTimeOpt = optional<string>(endTime);
            }

            if (strcmp(roomId, NULL_TAG) != 0)
            {
                roomIdOpt = optional<int>(stoi(roomId));
            }

            if (strcmp(userId, NULL_TAG) != 0)
            {
                userIdOpt = optional<int>(stoi(userId));
            }

            if (strcmp(states, NULL_TAG) != 0)
            {
                vector<string> statesVec;
                char *token = strtok(states, " ");
                while (token != NULL)
                {
                    statesVec.push_back(token);
                    token = strtok(NULL, " ");
                }
                statesOpt = optional<vector<string>>(statesVec);
            }

            vector<Item> items = itemModel.searchItem(SearchItemDto{userIdOpt, itemNameOpt, startTimeOpt, endTimeOpt, roomIdOpt, statesOpt});

            sprintf(response, "%d\n%d\n", SEARCH_ITEM_RES, SUCCESS);
            log_send_msg(response);
            sendItemList(items);
        }
        else
        {
            printf("[-]Invalid search item by name protocol! %s\n", payload);
            sprintf(response, "%d\n%d\n", SEARCH_ITEM_RES, FAIL);
            log_send_msg(response);
        }
    }
    catch (const exception &e)
    {
        printf("Exception caught in searchItem: %s\n", e.what());
        sprintf(response, "%d\n%d\n", SEARCH_ITEM_RES, FAIL);
        log_send_msg(response);
    }
}

void ItemHandler::updateItem(char *payload)
{
    char response[50], buyNowPrice[20];
    int userId, itemId;
    try
    {
        int noargs = sscanf(payload, "%d\n%d\n%s\n", &userId, &itemId, buyNowPrice);
        if (noargs == 3)
        {

            optional<double> buyNowPriceOpt;

            if (strcmp(buyNowPrice, NULL_TAG) != 0)
            {
                buyNowPriceOpt = optional<double>(stod(buyNowPrice));
            }

            int result = itemModel.updateItem(UpdateItemDto{buyNowPriceOpt, userId, itemId});

            sprintf(response, "%d\n%d\n", UPDATE_ITEM_RES, result);
            log_send_msg(response);
        }
        else
        {
            printf("[-]Invalid update item by name protocol! %s\n", payload);
            sprintf(response, "%d\n%d\n", UPDATE_ITEM_RES, FAIL);
            log_send_msg(response);
        }
    }
    catch (const exception &e)
    {
        printf("Exception caught in searchItem: %s\n", e.what());
        sprintf(response, "%d\n%d\n", UPDATE_ITEM_RES, FAIL);
        log_send_msg(response);
    }
}

void ItemHandler::deleteItem(char *buf)
{
    char response[50];
    int itemId, userId;
    try
    {
        int noargs = sscanf(buf, "%d\n%d\n", &itemId, &userId);
        if (noargs == 2)
        {
            int result = itemModel.deleteItem(itemId, userId);
            sprintf(response, "%d\n%d\n", DELETE_ITEM_RES, result);
        }
        else
        {
            printf("[-]Invalid delete item protocol! %s\n", buf);
            sprintf(response, "%d\n%d\n", DELETE_ITEM_RES, FAIL);
        }
    }
    catch (const exception &e)
    {
        printf("Exception caught in deleteItem: %s\n", e.what());
        sprintf(response, "%d\n%d\n", DELETE_ITEM_RES, FAIL);
    }
    log_send_msg(response);
}

void ItemHandler::createItem(char *buf)
{

    char itemName[MAX_ROOM_NAME], response[50];
    int ownerId;
    double buyNowPrice;
    try
    {
        int noargs = sscanf(buf, "%s\n%lf\n%d\n", itemName, &buyNowPrice, &ownerId);

        if (noargs == 3)
        {

            CreateItemDto request = {itemName, buyNowPrice, ownerId};
            int result = itemModel.createItem(request);
            sprintf(response, "%d\n%d\n", CREATE_ITEM_RES, result);
        }
        else
        {
            printf("[-]Invalid create item protocol! %s\n", buf);
            sprintf(response, "%d\n%d\n", CREATE_ITEM_RES, FAIL);
        }
    }
    catch (const exception &e)
    {
        printf("Exception caught in createItem: %s\n", e.what());
        sprintf(response, "%d\n%d\n", CREATE_ITEM_RES, FAIL);
    }
    log_send_msg(response);
}

void ItemHandler::sendItemList(vector<Item> items)
{
    char sendline[MAXLINE];
    memset(sendline, 0, MAXLINE);

    for (int i = 0; i < items.size(); i++)
    {
        Item item = items[i];
        string encodedItemName = encodeSpace(item.name);
        string encodedOwnerName = encodeSpace(item.ownerName);
        string encodedRoomName = encodeSpace(item.roomName.value_or("NULL"));
        string convertedStartTime = item.startTime.has_value() ? convertDateTimeFormat(item.startTime.value()) : "NULL";
        string convertedEndTime = item.endTime.has_value() ? convertDateTimeFormat(item.endTime.value()) : "NULL";
        sprintf(sendline, "%d %s %s %s %f %s %f %d %s %s %s\n", item.itemId, encodedItemName.c_str(), convertedStartTime.c_str(), convertedEndTime.c_str(), item.currentPrice, item.state.c_str(), item.buyNowPrice, item.ownerId, item.roomId.has_value() ? to_string(item.roomId.value()).c_str() : "NULL", encodedRoomName.c_str(), encodedOwnerName.c_str());
        log_send_msg(sendline);
        memset(sendline, 0, MAXLINE);
    }
    printf("[+]Send completely!\n");
}
