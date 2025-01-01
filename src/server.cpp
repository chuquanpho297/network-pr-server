#include <bits/stdc++.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include "database/mysql_connector.h"
#include "enums.h"
#include "config.h"
#include "user.h"
#include "item.h"
#include "room_handler.h"
#include "user_handler.h"
#include "handler.h"

using namespace std;

int socketfd;
int child_process_running = 1;
string END = "End\n";

void sig_chld(int signo);
void initServer();
void log_recv_msg(string client_ip, int client_port, string buf, UserRequest req);

int main(int argc, char **argv)
{
    signal(SIGCHLD, sig_chld);

    int listenfd, connfd, n, cliUserId = -1, cliRoomId = -1;
    pid_t childpid;
    socklen_t clilen;
    char buf[MAXLINE];
    struct sockaddr_in cliaddr, servaddr;
    initServer();
    for (;;)
    {
        clilen = sizeof(cliaddr);
        // accept a connection
        connfd = accept(socketfd, (struct sockaddr *)&cliaddr, &clilen);
        if (connfd < 0)
        {
            exit(1);
        }
        cout << "\n[+]" << inet_ntoa(cliaddr.sin_addr) << ":" << ntohs(cliaddr.sin_port) << " - Connection accepted. Received request..." << endl;

        if ((childpid = fork()) == 0) // if it’s 0, it’s child process
        {
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(cliaddr.sin_addr), client_ip, INET_ADDRSTRLEN); // convert ip number from binary to string
            int client_port = ntohs(cliaddr.sin_port);                           // network order to host short

            // close listening socket
            close(socketfd);

            // Initialize MySQL operations
            MySQLOperations mysqlOps;
            if (!mysqlOps.connect(DB_HOST, DB_USER, DB_PASS, DB_NAME))
            {
                cout << "[-]Failed to connect to MySQL database\n"
                     << endl;
                exit(1);
            }
            else
                cout << "[+]Connected to Database!\n";

            ItemHandler &itemHandler = ItemHandler::getInstance(connfd, client_ip, client_port, &mysqlOps);
            RoomHandler &roomHandler = RoomHandler::getInstance(connfd, client_ip, client_port, &mysqlOps);
            UserHandler &userHandler = UserHandler::getInstance(connfd, client_ip, client_port, &mysqlOps);

            while (child_process_running)
            {
                n = recv(connfd, buf, MAXLINE, 0);

                if (n < 0)
                {
                    perror("Read error");
                    exit(1);
                }
                if (n == 0)
                {
                    cout << "[+]" << client_ip << ":" << client_port << " - Disconnected" << endl;
                    exit(0);
                }

                buf[n] = '\0';

                char *newline_pos = strchr(buf, '\n');

                if (newline_pos == NULL)
                {
                    perror("Invalid protocol");
                    exit(1);
                }

                *newline_pos = '\0';
                char *cmd_str = buf;
                char *payload = newline_pos + 1;

                int cmd = atoi(cmd_str);

                log_recv_msg(client_ip, client_port, payload, static_cast<UserRequest>(cmd));

                switch (cmd)
                {
                case REGISTER_REQ:
                {
                    userHandler.registerUser(payload);
                    break;
                }
                case LOGIN_REQ:
                {
                    userHandler.login(payload);
                    break;
                }
                case LOGOUT_REQ:
                {
                    userHandler.logout(payload);
                    break;
                }
                case CREATE_ROOM_REQ:
                {
                    roomHandler.createRoom(payload);
                    break;
                }
                case VIEW_ROOMS_REQ:
                {
                    roomHandler.viewRooms();
                    break;
                }
                case VIEW_ROOMS_OWNED_REQ:
                {
                    roomHandler.viewRoomsOwned(payload);
                    break;
                }
                case JOIN_ROOM_REQ:
                {
                    roomHandler.joinRoom(payload, cliRoomId);
                    break;
                }
                case VIEW_ALL_ITEMS_REQ:
                {
                    itemHandler.viewAllItems();
                    break;
                }
                case VIEW_ITEMS_IN_ROOM_REQ:
                {
                    itemHandler.viewItemsInRoom(payload);
                    break;
                }
                case SEARCH_ITEM_REQ:
                {
                    itemHandler.searchItem(payload);
                    break;
                }
                case CREATE_ITEM_REQ:
                {
                    itemHandler.createItem(payload);
                    break;
                }
                case VIEW_OWNED_ITEMS_REQ:
                {
                    itemHandler.viewOwnedItems(payload);
                    break;
                }
                case UPDATE_ITEM_REQ:
                {
                    itemHandler.updateItem(payload);
                    break;
                }
                case DELETE_ITEM_REQ:
                {
                    itemHandler.deleteItem(payload);
                    break;
                }
                default:
                {
                    char response[50];
                    printf("[-]Invalid protocol: wrong command code\n\n");
                    sprintf(response, "%d\n", WRONG_COMMAND_RES);
                    itemHandler.log_send_msg(response, static_cast<ServerResponse>(-1));
                }
                }
                itemHandler.log_send_msg(END);
            }
            mysqlOps.disconnect();
            close(connfd);
        }
    }
}

void log_recv_msg(string client_ip, int client_port, string payload, UserRequest req)
{
    cout << "[+]Received message from " << client_ip << ":" << client_port << "\n"
         << req << "\n"
         << payload << endl;
}

void sig_chld(int signo)
{
    pid_t pid;
    int stat;

    // Add errno save/restore to prevent interference with other system calls
    int saved_errno = errno;

    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
    {
        cout << "[+]Child " << pid << " terminated with status " << stat << endl;
    }

    errno = saved_errno;
    return;
}

void initServer()
{
    struct sockaddr_in servaddr;
    // creation of the socket
    // If socketfd<0 there was an error in the creation of the socket
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0)
    {
        cout << "[-]Error in creating the socket" << endl;
        exit(1);
    }
    cout << "[+]Server Socket is created." << endl;

    // allow port reuse
    int opt = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        cout << "[-]Error in setting socket options" << endl;
        exit(1);
    }

    // preparation of the socket address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    // bind the socket
    if (bind(socketfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        cout << "[-]Error in binding." << endl;
        exit(1);
    }
    // listen to the socket by creating a connection queue, then wait for clients
    if (listen(socketfd, LISTENQ) == 0)
    {
        cout << "[+]Listening...." << endl;
    }
    else
    {
        cout << "[-]Error in binding." << endl;
    }
}