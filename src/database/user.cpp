#include <iostream>
#include <string>
#include "mysql_connector.h"
#include "user.h"

using namespace std;

int UserModel::registerUser(string username, string password)
{
    // Check if the username already exists
    string checkSql = "SELECT * FROM user WHERE name = '" + username + "';";
    cout << "SQL query: " << checkSql << '\n';
    sql::ResultSet *checkRes = mysqlOps->stmt->executeQuery(checkSql);

    if (checkRes && checkRes->next())
    {
        cout << "[-]Username already exists." << endl;
        delete checkRes;
        return 2; // Already existed
    }
    delete checkRes;

    string sql = "INSERT INTO user(name, password) VALUES ('" + username + "','" + password + "');";
    bool res = mysqlOps->stmt->executeUpdate(sql);
    cout << "SQL query: " << sql << '\n';
    if (res > 0)
        return 1; // SUCCESS
    else
        return 0; // FAIL
}

int UserModel::login(string username, string password, int &userId, optional<int> &roomId)
{
    // Verify username and password
    string sql = "SELECT user_id, room_id FROM user WHERE name = '" + username + "' AND password = '" + password + "';";
    cout << "SQL query: " << sql << '\n';
    sql::ResultSet *res = mysqlOps->stmt->executeQuery(sql);
    if (res && res->next())
    {
        userId = res->getInt("user_id");
        if (res->isNull("room_id"))
        {
            roomId = std::nullopt;
        }
        else
        {
            roomId = optional<int>(res->getInt("room_id"));
        }
        delete res;

        return 1; // SUCCESS
    }
    else
    {
        delete res;
        return 3; // Invalid username or password
    }
}

int UserModel::logout(int userId)
{
    string sql = "UPDATE user SET loggin = 0 WHERE user_id = " + to_string(userId) + ";";
    cout << "SQL query: " << sql << '\n';
    bool res = mysqlOps->stmt->executeUpdate(sql);
    if (res > 0)
        return 1; // SUCCESS
    else
        return 0; // FAIL
}