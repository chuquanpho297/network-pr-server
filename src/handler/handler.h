#ifndef HANDLER_H
#define HANDLER_H

#include <string>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <enums.h>

using namespace std;

class Handler
{
public:
    Handler(int connfd, string client_ip, int client_port)
        : connfd(connfd), client_ip(client_ip), client_port(client_port) {}
    virtual ~Handler() {}

    void log_send_msg(string endResponse, ServerResponse res);
    void log_send_msg(string endResponse);

protected:
    int connfd;
    string client_ip;
    int client_port;
};

#endif // HANDLER_H