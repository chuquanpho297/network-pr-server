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

private:
    RoomModel(MySQLOperations *mysqlOps) : mysqlOps(mysqlOps) {}
    ~RoomModel() {}
    RoomModel(const RoomModel &) = delete;
    RoomModel &operator=(const RoomModel &) = delete;
    Room parseResultSet(sql::ResultSet *res);

    MySQLOperations *mysqlOps;
};

#endif // ROOM_H