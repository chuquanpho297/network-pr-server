#include <string>
#include "handler.h"
#include <room.h>

using namespace std;

#ifndef ROOM_HANDLER_H
#define ROOM_HANDLER_H

struct Room;
struct RoomLog;
struct UserLog;
class RoomModel;

struct CreateRoomDto
{
    int userId;
    string roomName;
    string startTime;
    string endTime;
};

class RoomHandler : public Handler
{
public:
    static RoomHandler &getInstance(int connfd, string client_ip, int client_port, MySQLOperations *mysqlOps)
    {
        static RoomHandler instance(connfd, client_ip, client_port, mysqlOps);
        return instance;
    }

    void createRoom(char *payload);
    void viewRooms();
    void viewRoomsOwned(char *payload);
    void sendRoomList(vector<Room> rooms);
    void placeBid(char *payload);
    void acceptRejectItem(char *payload);
    void viewRoomLog(char *payload);
    void leaveRoom(char *payload);
    void joinRoom(char *payload);
    void buyNow(char *payload);
    void placeItemInRoom(char *payload);
    void deleteItemFromRoom(char *payload);
    void viewUserLog(char *payload);
    void sendRoomLogList(vector<RoomLog> roomLogs);
    void sendUserLogList(vector<UserLog> userLogs);

    RoomModel &roomModel;

private:
    RoomHandler(int connfd, string client_ip, int client_port, MySQLOperations *mysqlOps);
    ~RoomHandler() {}
    RoomHandler(const RoomHandler &) = delete;
    RoomHandler &operator=(const RoomHandler &) = delete;

    MySQLOperations *mysqlOps;
};

#endif // ROOM_HANDLER_H
