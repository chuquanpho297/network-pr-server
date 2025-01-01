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

    string sql = "INSERT INTO room(name, owner_id) VALUES ('" + dto.roomName + "'," + to_string(dto.userId) + ");";
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
    string sql = "SELECT r.room_id, r.name, r.owner_id, u.name AS owner_name, "
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

Room RoomModel::parseResultSet(sql::ResultSet *res)
{
    Room room;
    room.roomId = res->getInt("room_id");
    room.name = res->getString("name");
    room.ownerId = res->getInt("owner_id");
    room.ownerName = res->getString("owner_name");
    room.totalItems = res->getInt("totalItems");
    room.totalParticipants = res->getInt("totalParticipants");

    return room;
}