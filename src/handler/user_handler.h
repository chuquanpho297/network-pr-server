#ifndef USER_HANDLER_H
#define USER_HANDLER_H

#include <string>
#include <optional>
#include "mysql_connector.h"
#include "handler.h"
#include <user.h>

using namespace std;

struct User;
class UserModel;

class UserHandler : public Handler
{
public:
    static UserHandler &getInstance(int connfd, string client_ip, int client_port, MySQLOperations *mysqlOps)
    {
        static UserHandler instance(connfd, client_ip, client_port, mysqlOps);
        return instance;
    }

    void registerUser(char *payload);
    void login(char *payload);
    void logout(char *payload);

    UserModel &userModel;

private:
    UserHandler(int connfd, string client_ip, int client_port, MySQLOperations *mysqlOps);
    ~UserHandler() {}
    UserHandler(const UserHandler &) = delete;
    UserHandler &operator=(const UserHandler &) = delete;

    MySQLOperations *mysqlOps;
};

#endif // USER_HANDLER_H