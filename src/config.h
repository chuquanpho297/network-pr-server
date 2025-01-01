#ifndef CONFIG_H
#define CONFIG_H

#define DB_HOST "tcp://127.0.0.1:3306"
#define DB_USER "root"
#define DB_PASS "1"
#define DB_NAME "auctionDb"

#define BUFFER_SIZE 4096
#define MAX_CLIENTS 8
#define MAX_USERNAME 255
#define MAX_PASSWORD 255
#define MAX_ROOM_NAME 255
#define NULL_TAG "NULL"

#define MAXLINE 4096   // max text line length
#define SERV_PORT 3000 // port
#define LISTENQ 8      // maximum number of client connections
#endif                 // CONFIG_H