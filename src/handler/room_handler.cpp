#include "room_handler.h"
#include <config.h>
#include <optional>
#include <string.h>
#include <algorithm>
#include <util.h>
using namespace std;

RoomHandler::RoomHandler(int connfd, string client_ip, int client_port, MySQLOperations *mysqlOps)
    : Handler(connfd, client_ip, client_port), mysqlOps(mysqlOps), roomModel(RoomModel::getInstance(mysqlOps)) {}

void RoomHandler::createRoom(char *payload)
{
    char roomName[MAX_ROOM_NAME], startTime[20], endTime[20], response[50];
    int userId;
    try
    {
        int noargs = sscanf(payload, "%d\n%s\n%s\n%s\n", &userId, roomName, startTime, endTime);
        if (noargs == 4)
        {
            int result = roomModel.createRoom(CreateRoomDto{userId, roomName, startTime, endTime});
            sprintf(response, "%d\n%d\n", CREATE_ROOM_RES, result);
        }
        else
        {
            printf("[-]Invalid create room protocol! %s\n", payload);
            sprintf(response, "%d\n%d\n", CREATE_ROOM_RES, FAIL);
        }
    }
    catch (const exception &e)
    {
        printf("Exception caught in createRoom: %s\n", e.what());
        sprintf(response, "%d\n%d\n", CREATE_ROOM_RES, FAIL);
    }
    log_send_msg(response);
}

void RoomHandler::viewRooms()
{
    char response[50];
    try
    {
        vector<Room> rooms = roomModel.getRooms(optional<int>(), optional<string>(" AND r.end_time > NOW()"));
        sprintf(response, "%d\n%d\n", VIEW_ROOMS_RES, SUCCESS);
        log_send_msg(response);
        sendRoomList(rooms);
    }
    catch (const exception &e)
    {
        printf("Exception caught in viewRooms: %s\n", e.what());
        sprintf(response, "%d\n%d\n", VIEW_ROOMS_RES, FAIL);
        log_send_msg(response);
    }
}

void RoomHandler::viewRoomsOwned(char *payload)
{
    char response[50], sendline[MAXLINE];
    int userId;
    try
    {
        int noargs = sscanf(payload, "%d\n", &userId);
        if (noargs == 1)
        {
            printf("[+]View rooms owned by user %d\n", userId);
            vector<Room> rooms = roomModel.getRooms(optional<int>(userId), optional<string>());
            sprintf(response, "%d\n%d\n", VIEW_ROOMS_OWNED_RES, SUCCESS);
            log_send_msg(response);
            sendRoomList(rooms);
        }
        else
        {
            printf("[-]Invalid view rooms owned protocol! %s\n", payload);
            sprintf(response, "%d\n%d\n", VIEW_ROOMS_OWNED_RES, FAIL);
            log_send_msg(response);
        }
    }
    catch (const exception &e)
    {
        printf("Exception caught in viewRoomsOwned: %s\n", e.what());
        sprintf(response, "%d\n%d\n", VIEW_ROOMS_OWNED_RES, FAIL);
        log_send_msg(response);
    }
}

void RoomHandler::joinRoom(char *payload)
{
    char response[50];
    int userId, roomId;
    try
    {
        int noargs = sscanf(payload, "%d\n%d\n", &userId, &roomId);
        if (noargs == 2)
        {
            int result = roomModel.joinRoom(userId, roomId);
            sprintf(response, "%d\n%d\n", JOIN_ROOM_RES, result);
        }
        else
        {
            printf("[-]Invalid join room protocol! %s\n", payload);
            sprintf(response, "%d\n%d\n", JOIN_ROOM_RES, FAIL);
        }
    }
    catch (const exception &e)
    {
        printf("Exception caught in joinRoom: %s\n", e.what());
        sprintf(response, "%d\n%d\n", JOIN_ROOM_RES, FAIL);
    }
    log_send_msg(response);
}

void RoomHandler::placeBid(char *payload)
{
    char response[50];
    int userId, itemId;
    double bidPrice;
    try
    {
        int noargs = sscanf(payload, "%d\n%d\n%lf\n", &userId, &itemId, &bidPrice);
        if (noargs == 3)
        {
            int result = roomModel.placeBid(userId, itemId, bidPrice);
            sprintf(response, "%d\n%d\n", PLACE_BID_RES, result);
        }
        else
        {
            printf("[-]Invalid place bid protocol! %s\n", payload);
            sprintf(response, "%d\n%d\n", PLACE_BID_RES, FAIL);
        }
    }
    catch (const exception &e)
    {
        printf("Exception caught in placeBid: %s\n", e.what());
        sprintf(response, "%d\n%d\n", PLACE_BID_RES, FAIL);
    }
    log_send_msg(response);
}

void RoomHandler::acceptRejectItem(char *payload)
{
    char response[50];
    int itemId, roomId, confirmCode;
    try
    {
        int noargs = sscanf(payload, "%d\n%d\n%d\n", &itemId, &roomId, &confirmCode);
        if (noargs == 3)
        {
            int result = roomModel.acceptRejectItem(itemId, roomId, confirmCode);
            sprintf(response, "%d\n%d\n", ACCEPT_REJECT_ITEM_RES, result);
        }
        else
        {
            printf("[-]Invalid accept reject item protocol! %s\n", payload);
            sprintf(response, "%d\n%d\n", ACCEPT_REJECT_ITEM_RES, FAIL);
        }
    }
    catch (const exception &e)
    {
        printf("Exception caught in acceptRejectItem: %s\n", e.what());
        sprintf(response, "%d\n%d\n", ACCEPT_REJECT_ITEM_RES, FAIL);
    }
    log_send_msg(response);
}

void RoomHandler::viewRoomLog(char *payload)
{
    char response[50];
    int roomId;
    try
    {
        int noargs = sscanf(payload, "%d\n", &roomId);
        if (noargs == 1)
        {
            vector<RoomLog> roomLogs = roomModel.getRoomLog(roomId);
            sprintf(response, "%d\n%d\n", VIEW_ROOM_LOG_RES, SUCCESS);
            log_send_msg(response);
            sendRoomLogList(roomLogs);
        }
        else
        {
            printf("[-]Invalid view room log protocol! %s\n", payload);
            sprintf(response, "%d\n%d\n", VIEW_ROOM_LOG_RES, FAIL);
            log_send_msg(response);
        }
    }
    catch (const exception &e)
    {
        printf("Exception caught in viewRoomLog: %s\n", e.what());
        sprintf(response, "%d\n%d\n", VIEW_ROOM_LOG_RES, FAIL);
        log_send_msg(response);
    }
}

void RoomHandler::leaveRoom(char *payload)
{
    char response[50];
    int userId, roomId;
    try
    {
        int noargs = sscanf(payload, "%d\n%d\n", &userId, &roomId);
        if (noargs == 2)
        {
            int result = roomModel.leaveRoom(userId, roomId);
            sprintf(response, "%d\n%d\n", LEAVE_ROOM_RES, result);
        }
        else
        {
            printf("[-]Invalid leave room protocol! %s\n", payload);
            sprintf(response, "%d\n%d\n", LEAVE_ROOM_RES, FAIL);
        }
    }
    catch (const exception &e)
    {
        printf("Exception caught in leaveRoom: %s\n", e.what());
        sprintf(response, "%d\n%d\n", LEAVE_ROOM_RES, FAIL);
    }
    log_send_msg(response);
}

void RoomHandler::buyNow(char *payload)
{
    char response[50];
    int userId, itemId;
    try
    {
        int noargs = sscanf(payload, "%d\n%d\n", &userId, &itemId);
        if (noargs == 2)
        {
            int result = roomModel.buyNow(userId, itemId);
            sprintf(response, "%d\n%d\n", BUY_NOW_RES, result);
        }
        else
        {
            printf("[-]Invalid buy now protocol! %s\n", payload);
            sprintf(response, "%d\n%d\n", BUY_NOW_RES, FAIL);
        }
    }
    catch (const exception &e)
    {
        printf("Exception caught in buyNow: %s\n", e.what());
        sprintf(response, "%d\n%d\n", BUY_NOW_RES, FAIL);
    }
}

void RoomHandler::placeItemInRoom(char *payload)
{
    char response[50];
    int userId, itemId, roomId;
    try
    {
        int noargs = sscanf(payload, "%d\n%d\n%d\n", &userId, &itemId, &roomId);
        if (noargs == 3)
        {
            int result = roomModel.placeItemInRoom(userId, itemId, roomId);
            sprintf(response, "%d\n%d\n", PLACE_ITEM_IN_ROOM_RES, result);
        }
        else
        {
            printf("[-]Invalid place item in room protocol! %s\n", payload);
            sprintf(response, "%d\n%d\n", PLACE_ITEM_IN_ROOM_RES, FAIL);
        }
    }
    catch (const exception &e)
    {
        printf("Exception caught in placeItemInRoom: %s\n", e.what());
        sprintf(response, "%d\n%d\n", PLACE_ITEM_IN_ROOM_RES, FAIL);
    }
    log_send_msg(response);
}

void RoomHandler::deleteItemFromRoom(char *payload)
{
    char response[50];
    int itemId, roomId, userId;
    try
    {
        int noargs = sscanf(payload, "%d\n%d\n%d\n", &userId, &itemId, &roomId);
        if (noargs == 2)
        {
            int result = roomModel.deleteItemFromRoom(userId, itemId, roomId);
            sprintf(response, "%d\n%d\n", DELETE_ITEM_IN_ROOM_REQ, result);
        }
        else
        {
            printf("[-]Invalid delete item from room protocol! %s\n", payload);
            sprintf(response, "%d\n%d\n", DELETE_ITEM_IN_ROOM_REQ, FAIL);
        }
    }
    catch (const exception &e)
    {
        printf("Exception caught in deleteItemFromRoom: %s\n", e.what());
        sprintf(response, "%d\n%d\n", DELETE_ITEM_IN_ROOM_REQ, FAIL);
    }
    log_send_msg(response);
}

void RoomHandler::viewUserLog(char *payload)
{
    char response[50];
    int roomId, itemId;
    try
    {
        int noargs = sscanf(payload, "%d\n%d\n", &roomId, &itemId);
        if (noargs == 2)
        {
            vector<UserLog> userLogs = roomModel.getUserLog(roomId, itemId);
            sprintf(response, "%d\n%d\n", VIEW_USER_LOG_RES, SUCCESS);
            log_send_msg(response);
            sendUserLogList(userLogs);
        }
        else
        {
            printf("[-]Invalid view user log protocol! %s\n", payload);
            sprintf(response, "%d\n%d\n", VIEW_USER_LOG_RES, FAIL);
            log_send_msg(response);
        }
    }
    catch (const exception &e)
    {
        printf("Exception caught in viewUserLog: %s\n", e.what());
        sprintf(response, "%d\n%d\n", VIEW_USER_LOG_RES, FAIL);
        log_send_msg(response);
    }
}

void RoomHandler::sendRoomList(vector<Room> rooms)
{
    char sendline[MAXLINE];
    for (int i = 0; i < rooms.size(); i++)
    {
        Room room = rooms[i];
        string encodedRoomName = encodeSpace(room.name);
        string encodedOwnerName = encodeSpace(room.ownerName);
        string convertedStartTime = convertDateTimeFormat(room.startTime);
        string convertedEndTime = convertDateTimeFormat(room.endTime);

        sprintf(sendline, "%d %s %d %s %d %d %s %s\n",
                room.roomId,
                encodedRoomName.c_str(),
                room.ownerId,
                encodedOwnerName.c_str(),
                room.totalItems,
                room.totalParticipants,
                convertedStartTime.c_str(),
                convertedEndTime.c_str());
        log_send_msg(sendline);
        memset(sendline, 0, MAXLINE);
    }
}

void RoomHandler::sendRoomLogList(vector<RoomLog> roomLogs)
{
    char sendline[MAXLINE];
    for (int i = 0; i < roomLogs.size(); i++)
    {
        RoomLog roomLog = roomLogs[i];
        string convertedTime = convertDateTimeFormat(roomLog.time);
        string encodedItemName = encodeSpace(roomLog.itemName);
        sprintf(sendline, "%d %d %s %d %s %lf %s\n",
                roomLog.roomLogId,
                roomLog.itemId,
                encodedItemName.c_str(),
                roomLog.roomId,
                roomLog.status.c_str(),
                roomLog.buyNowPrice,
                convertedTime.c_str());
        log_send_msg(sendline);
        memset(sendline, 0, MAXLINE);
    }
}

void RoomHandler::sendUserLogList(vector<UserLog> userLogs)
{
    char sendline[MAXLINE];
    for (int i = 0; i < userLogs.size(); i++)
    {
        UserLog userLog = userLogs[i];
        string convertedTime = convertDateTimeFormat(userLog.time);
        string encodedUserName = encodeSpace(userLog.userName);
        sprintf(sendline, "%d %d %d %d %s %s %s %s\n",
                userLog.logId,
                userLog.userId,
                userLog.itemId,
                userLog.roomId,
                encodedUserName.c_str(),
                to_string(userLog.bidPrice).c_str(),
                convertedTime.c_str(), userLog.status.c_str());
        log_send_msg(sendline);
        memset(sendline, 0, MAXLINE);
    }
}