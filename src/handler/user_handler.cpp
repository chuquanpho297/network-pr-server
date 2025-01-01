#include "user_handler.h"
#include <config.h>

UserHandler::UserHandler(int connfd, string client_ip, int client_port, MySQLOperations *mysqlOps)
    : Handler(connfd, client_ip, client_port), mysqlOps(mysqlOps), userModel(UserModel::getInstance(mysqlOps)) {}

void UserHandler::registerUser(char *payload)
{
    char username[MAX_USERNAME], password[MAX_PASSWORD], response[50];
    try
    {
        int noargs = sscanf(payload, "%s\n%s\n", username, password);
        if (noargs == 2)
        {
            int result = userModel.registerUser(username, password);

            sprintf(response, "%d\n%d\n", REGISTER_RES, result);
        }
        else
        {
            printf("[-]Invalid register protocol! %s\n", payload);
            sprintf(response, "%d\n%d\n", REGISTER_RES, FAIL);
        }
    }
    catch (const exception &e)
    {
        printf("Exception caught in registerUser: %s\n", e.what());
        sprintf(response, "%d\n%d\n", REGISTER_RES, FAIL);
    }
    log_send_msg(response);
}

void UserHandler::login(char *payload)
{
    char username[MAX_USERNAME], password[MAX_PASSWORD], response[50];
    try
    {
        int noargs = sscanf(payload, "%s\n%s\n", username, password);
        if (noargs == 2)
        {
            int userId, roomId;
            int result = userModel.login(username, password, userId, roomId);
            if (result == SUCCESS)
            {
                sprintf(response, "%d\n%d\n%d %d\n", LOGIN_RES, result, userId, roomId);
            }
            else
            {
                sprintf(response, "%d\n%d\n", LOGIN_RES, result);
            }
        }
        else
        {
            printf("[-]Invalid login protocol! %s\n", payload);
            sprintf(response, "%d\n%d\n", LOGIN_RES, FAIL);
        }
    }
    catch (const exception &e)
    {
        printf("Exception caught in login: %s\n", e.what());
        sprintf(response, "%d\n%d\n", LOGIN_RES, FAIL);
    }
    log_send_msg(response);
}

void UserHandler::logout(char *payload)
{
    char response[50];
    int user_id;
    try
    {
        int noargs = sscanf(payload, "%d\n", &user_id);
        if (noargs == 1)
        {
            int result = userModel.logout(user_id);
            sprintf(response, "%d\n%d\n", LOGOUT_RES, result);
        }
        else
        {
            printf("[-]Invalid logout protocol! %s\n", payload);
            sprintf(response, "%d\n%d\n", LOGOUT_RES, FAIL);
        }
    }
    catch (const exception &e)
    {
        printf("Exception caught in logout: %s\n", e.what());
        sprintf(response, "%d\n%d\n", LOGOUT_RES, FAIL);
    }
    log_send_msg(response);
}
