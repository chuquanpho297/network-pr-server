#ifndef MYSQL_CONNECTOR_H
#define MYSQL_CONNECTOR_H

#include <string>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

class MySQLOperations
{
public:
    MySQLOperations();
    ~MySQLOperations();

    bool connect(const std::string &ipAddress, const std::string &username, const std::string &password, const std::string &database);
    void disconnect();

    sql::Statement *stmt;
private:
    sql::mysql::MySQL_Driver *driver;
    sql::Connection *conn;
};

#endif // MYSQL_CONNECTOR_H