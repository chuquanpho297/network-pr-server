#ifndef USER_H
#define USER_H

#include <string>
#include "mysql_connector.h"
#include <optional>

using namespace std;

struct User
{
    int userId;
    char username[255];
    char password[255];
    int roomId;
};

class UserModel
{
public:
    static UserModel &getInstance(MySQLOperations *mysqlOps)
    {
        static UserModel instance(mysqlOps);
        return instance;
    }

    int registerUser(string username, string password);
    int login(string username, string password, int &userId, optional<int> &roomId);
    int logout(int userId);

private:
    UserModel(MySQLOperations *mysqlOps) : mysqlOps(mysqlOps) {}
    ~UserModel() {}
    UserModel(const UserModel &) = delete;
    UserModel &operator=(const UserModel &) = delete;

    MySQLOperations *mysqlOps;
};

#endif // USER_H