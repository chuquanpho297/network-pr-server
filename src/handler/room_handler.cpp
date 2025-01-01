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
        int noargs = sscanf(payload, "%d\n%s\n", &userId, roomName);
        if (noargs == 2)
        {
            int result = roomModel.createRoom(CreateRoomDto{userId, roomName});
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
        vector<Room> rooms = roomModel.getRooms(optional<int>(), optional<string>());
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

void RoomHandler::joinRoom(char *payload, int &cliRoomId)
{
    char response[50];
    int userId, roomId;
    try
    {
        int noargs = sscanf(payload, "%d\n%d\n", &userId, &roomId);
        if (noargs == 2)
        {
            int result = roomModel.joinRoom(userId, roomId);
            cliRoomId = roomId;
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

void RoomHandler::sendRoomList(vector<Room> rooms)
{
    char sendline[MAXLINE];
    for (int i = 0; i < rooms.size(); i++)
    {
        Room room = rooms[i];
        string encodedRoomName = encodeSpace(room.name);
        string encodedOwnerName = encodeSpace(room.ownerName);
        printf("[+]Room name: %s (encoded: %s)\n", room.name.c_str(), encodedRoomName.c_str());
        printf("[+]Owner name: %s (encoded: %s)\n", room.ownerName.c_str(), encodedOwnerName.c_str());

        sprintf(sendline, "%d %s %d %s %d %d\n",
                room.roomId,
                encodedRoomName.c_str(),
                room.ownerId,
                encodedOwnerName.c_str(),
                room.totalItems,
                room.totalParticipants);
        log_send_msg(sendline);
        memset(sendline, 0, MAXLINE);
    }
}