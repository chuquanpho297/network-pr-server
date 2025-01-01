#include "handler.h"

void Handler::log_send_msg(string endResponse, ServerResponse res)
{
    int n = send(connfd, endResponse.c_str(), endResponse.size(), 0);
    if (n < 0)
    {
        perror("Send error");
        exit(1);
    }
    cout << "[+]Sent message to " << client_ip << ":" << client_port << "\n"
         << res << "\n"
         << endResponse << '\n';
}

void Handler::log_send_msg(string endResponse)
{
    int n = send(connfd, endResponse.c_str(), endResponse.size(), 0);
    if (n < 0)
    {
        perror("Send error");
        exit(1);
    }
    cout << "[+]Sent message to " << client_ip << ":" << client_port << "\n"
         << endResponse << '\n';
}
