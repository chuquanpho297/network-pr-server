#include <bits/stdc++.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "enums.h"

#define MAXLINE 4096   // max text line length
#define SERV_PORT 3000 // port

using namespace std;
// session data
int socketfd, loggedIn, user_id, room_id = -1;

void connectToServer(char *ip);
void displayMenu();
void displayUserMenu(string *username);
int login(string *username, int &user_id, int &room_id);
void _register();
void displayListMessage(int *socketfd);
bool isInteger(const string &s);
void logout(string username);
void createRoom();
void viewRooms();
void viewRoomsOwned();
void joinRoom();

int main(int argc, char **argv)
{
    loggedIn = 0;
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <server_address>\n", argv[0]);
        exit(1);
    }

    connectToServer(argv[1]);

    displayMenu();
    return 0;
}

void connectToServer(char *ip)
{
    struct sockaddr_in servaddr;
    // Create a socket for the client
    // If socketfd<0 there was an error in the creation of the socket
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("[-]Problem in creating the socket");
        exit(2);
    }
    cout << "[+]Client Socket is created." << endl;
    // Creation of the socket
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(SERV_PORT); // convert to big-endian order
    // Connection of the client to the socket
    if (connect(socketfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("[-]Problem in connecting to the server");
        exit(3);
    }
    cout << "[+]Connected to Server." << endl;
}

void displayMenu()
{
    string username;
    while (1)
    {
        if (!loggedIn)
        {
            printf("\n====================================\n");
            printf("Welcome to Online Auction Application\n");
            printf("1. Register\n");
            printf("2. Login\n");
            printf("3. Exit\n");
            printf("Enter your choice: ");

            string choice;
            cin >> choice;

            switch (choice[0])
            {
            case '1':
                _register();
                break;

            case '2':
                loggedIn = login(&username, user_id, room_id);
                break;

            case '3':
                printf("***Application Ended***\n");
                exit(0);
                break;
            default:
                printf("Invalid choice. Try again.\n");
                break;
            }
            printf("-----------------------------\n");
        }
        else
        {
            // display menu corresponding to each user mode
            switch (loggedIn)
            {
            default:
                displayUserMenu(&username);
            }
            printf("-----------------------------\n");
        }
    }
}

void displayUserMenu(string *username)
{
    cout << "Account: " << *username << "; ID: " << user_id;
    if (room_id != -1)
    {
        cout << "; Room ID: " << room_id;
    }
    cout << "\n=============Main Menu==============\n";
    printf("1. View rooms\n");
    printf("2. Create room\n");
    printf("3. View rooms owned\n");
    printf("4. Join room\n");
    printf("5. Logout\n");
    printf("Enter your choice: ");

    string choice;
    cin >> choice;

    switch (choice[0])
    {
    case '1':
        viewRooms();
        break;
    case '2':
        createRoom();
        break;
    case '3':
        viewRoomsOwned();
        break;
    case '4':
        joinRoom();
        break;
    case '5':
        logout(*username);
        break;
    default:
        printf("Invalid choice. Try again.\n");
        break;
    }
}

void _register()
{
    char username[30], password[30];
    char sendline[MAXLINE], recvline[MAXLINE];

    cout << "Enter your username: ";
    cin >> username;
    cout << "Enter your password: ";
    cin >> password;

    // store values in sendline
    sprintf(sendline, "%d\n%s %s\n", REGISTER_REQ, username, password);
    // send request to server with protocol: "REGISTER\n<username> <password>\n"
    send(socketfd, sendline, strlen(sendline), 0);

    recv(socketfd, recvline, MAXLINE, 0);

    int response, status;
    sscanf(recvline, "%d\n%d\n", &response, status);
    if (status == 1) // SUCCESS
    {
        printf("Register successfully!\nNow you can login with this new account.\n");
    }
    else if (status == 2) // ALREADY EXISTED
    {
        printf("This username already existed!!!\n");
    }
    else if (status == 0) // FAIL
    {
        printf("Registration failed!!!\n");
    }
    else
    {
        perror(recvline);
        exit(4);
    }
}

int login(string *user, int &user_id, int &room_id)
{
    char username[30], password[30];
    char sendline[MAXLINE], recvline[MAXLINE];

    cout << "Enter your username: ";
    cin >> username;
    cout << "Enter your password: ";
    cin >> password;

    // store values in sendline
    sprintf(sendline, "%d\n%s %s\n", LOGIN_REQ, username, password);
    // send request to server with protocol: "LOGIN\n<username> <password>\n"
    send(socketfd, sendline, strlen(sendline), 0);

    recv(socketfd, recvline, MAXLINE, 0);
    int status, id, room;
    sscanf(recvline, "%d %d %d", &status, &id, &room);
    if (status == SUCCESS)
    {
        printf("You have logged in successfully!\n");
        *user = username;
        user_id = id;
        room_id = room;
        return 1;
    }
    else if (status == FAIL)
    {
        printf("Wrong username or password!!!\n");
    }
    else if (status == 2)
    {
        printf("User already logged in!\n");
    }
    else
    {
        perror(recvline);
        exit(4);
    }
    return 0;
}

void logout(string username)
{
    char sendline[MAXLINE], recvline[MAXLINE];
    sprintf(sendline, "%d\n%d\n", LOGOUT_REQ, user_id);
    send(socketfd, sendline, strlen(sendline), 0);
    recv(socketfd, recvline, MAXLINE, 0);
    int status = recvline[0] - '0';
    if (status == SUCCESS)
    {
        loggedIn = 0;
        room_id = -1;
        printf("Logged out successfully.\n\n");
    }
    else
    {
        printf("Logout failed!!!\n");
    }
}

void displayListMessage(int *socketfd)
{
    char recvline[MAXLINE];
    int n;
    while ((n = recv(*socketfd, recvline, MAXLINE, 0)) > 0)
    {
        if (strcmp(recvline, "End"))
        {
            printf("%s", recvline);
            memset(recvline, 0, sizeof(recvline));
        }
        else
        {
            memset(recvline, 0, sizeof(recvline));
            break;
        }
    }
}
void viewRooms()
{
    char sendline[MAXLINE], recvline[MAXLINE];
    int n;

    sprintf(sendline, "%d\n", VIEW_ROOMS_REQ);
    send(socketfd, sendline, strlen(sendline), 0);

    recv(socketfd, recvline, MAXLINE, 0);
    int status = recvline[0] - '0';
    if (status == SUCCESS)
    {
        displayListMessage(&socketfd);
    }
    else if (status == FAIL)
    {
        printf("Fail to get list Rooms!\n");
    }
    else
    {
        perror(recvline);
        exit(4);
    }
}

void createRoom()
{
    char room_name[255];
    char sendline[MAXLINE], recvline[MAXLINE];

    cout << "Enter room name: ";
    cin >> room_name;

    // store values in sendline
    sprintf(sendline, "%d\n%d %s\n", CREATE_ROOM_REQ, user_id, room_name);
    // send request to server with protocol: "CREATE_ROOM\n<user_id> <room_name>\n"
    send(socketfd, sendline, strlen(sendline), 0);

    recv(socketfd, recvline, MAXLINE, 0);
    int status = recvline[0] - '0';
    if (status == 1) // SUCCESS
    {
        printf("Room created successfully!\n");
    }
    else if (status == 0) // FAIL
    {
        printf("Room creation failed!!!\n");
    }
    else if (status == 2) // Already existed
    {
        printf("Room name already existed!!!\n");
    }
    else
    {
        perror(recvline);
        exit(4);
    }
}

void viewRoomsOwned()
{
    char sendline[MAXLINE], recvline[MAXLINE];
    int n;

    sprintf(sendline, "%d\n%d\n", VIEW_ROOMS_OWNED_REQ, user_id);
    send(socketfd, sendline, strlen(sendline), 0);

    recv(socketfd, recvline, MAXLINE, 0);
    int status = recvline[0] - '0';
    if (status == SUCCESS)
    {
        displayListMessage(&socketfd);
    }
    else if (status == FAIL)
    {
        printf("Fail to get list of owned rooms!\n");
    }
    else
    {
        perror(recvline);
        exit(4);
    }
}

void joinRoom()
{
    if (room_id != -1)
    {
        printf("You are already in a room. Please Leave the current room!\n");
        return;
    }

    int room_id_input;
    cout << "Enter room ID to join: ";
    cin >> room_id_input;

    char sendline[MAXLINE], recvline[MAXLINE];
    sprintf(sendline, "%d\n%d %d\n", JOIN_ROOM_REQ, user_id, room_id_input);
    send(socketfd, sendline, strlen(sendline), 0);

    recv(socketfd, recvline, MAXLINE, 0);
    int status = recvline[0] - '0';
    if (status == SUCCESS)
    {
        room_id = room_id_input;
        printf("Joined room successfully!\n");
    }
    else
    {
        printf("Room does not exsit!\n");
    }
}