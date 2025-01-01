#ifndef ROOM_H
#define ROOM_H

#include <string>
#include <vector>
#include "mysql_connector.h"
#include <handler/room_handler.h>
#include <config.h>
#include "optional"

using namespace std;

struct Room
{
    int roomId;
    string name;
    int totalItems;
    int totalParticipants;
    int ownerId;
    string ownerName;
    string startTime;
    string endTime;
};

struct RoomLog
{
    int roomLogId;
    int itemId;
    string itemName;
    int roomId;
    string status;
    double buyNowPrice;
    string time;
};

struct UserLog
{
    int logId;
    int userId;
    int itemId;
    int roomId;
    string userName;
    double bidPrice;
    string time;
    string status;
};

class RoomModel
{
public:
    static RoomModel &getInstance(MySQLOperations *mysqlOps)
    {
        static RoomModel instance(mysqlOps);
        return instance;
    }

    int createRoom(const CreateRoomDto &request);
    vector<Room> getRooms(optional<int> userId, optional<string> condition);
    int joinRoom(int userId, int roomId);
    int placeBid(int userId, int itemId, double bidPrice);
    int buyNow(int userId, int itemId);
    int leaveRoom(int userId, int roomId);
    int placeItemInRoom(int userId, int itemId, int roomId);
    int acceptRejectItem(int userId, int itemId, int status);
    vector<RoomLog> getRoomLog(int roomId);
    vector<UserLog> getUserLog(int roomId, int itemId);
    int deleteItemFromRoom(int userId, int itemId, int roomId);

private:
    RoomModel(MySQLOperations *mysqlOps) : mysqlOps(mysqlOps)
    {
    }
    ~RoomModel() {}
    RoomModel(const RoomModel &) = delete;
    RoomModel &operator=(const RoomModel &) = delete;
    Room parseResultSet(sql::ResultSet *res);
    RoomLog parseRoomLogResultSet(sql::ResultSet *res);

    MySQLOperations *mysqlOps;
};

#endif // ROOM_H