#include "mysql_connector.h"
#include <string.h>

#include "mysql_connector.h"

MySQLOperations::MySQLOperations()
{
    driver = sql::mysql::get_mysql_driver_instance();
    conn = nullptr;
    stmt = nullptr;
}

MySQLOperations::~MySQLOperations()
{
    disconnect();
}

bool MySQLOperations::connect(const std::string &ipAddress, const std::string &username, const std::string &password, const std::string &database)
{
    try
    {
        conn = driver->connect(ipAddress, username, password);
        conn->setSchema(database);
        stmt = conn->createStatement();

        // Set the time zone
        stmt->execute("SET time_zone = '+07:00';");

        return true;
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "Error connecting to database: " << e.what() << std::endl;
        return false;
    }
}

void MySQLOperations::disconnect()
{
    if (stmt)
    {
        delete stmt;
        stmt = nullptr;
    }
    if (conn)
    {
        delete conn;
        conn = nullptr;
    }
}