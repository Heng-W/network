#ifndef NET_DB_CONNECTION_H
#define NET_DB_CONNECTION_H

#include <string>
#include <memory>
#include "util/common.h"

struct st_mysql;
struct st_mysql_res;

namespace net
{

struct DBInfo
{
    std::string hostName = "localhost";
    std::string userName;
    std::string password;
    std::string dbName;
    uint16_t dbPort = 3306;
};


class QueryResult
{
public:
    QueryResult(st_mysql_res* res): res_(res) {}
    ~QueryResult() { free(); }

    DISALLOW_COPY_AND_ASSIGN(QueryResult);

    QueryResult(QueryResult&& rhs): res_(rhs.res_) { rhs.res_ = nullptr; }
    QueryResult& operator=(QueryResult&& rhs)
    {
        if (this != &rhs)
        {
            free();
            res_ = rhs.res_;
            rhs.res_ = nullptr;
        }
        return *this;
    }

    explicit operator bool() const { return res_ != nullptr; }

    int rows() const;
    int fields() const;

    char** fetchRow();

private:
    void free();

    st_mysql_res* res_;
};


class DBConnection
{
public:
    DBConnection(const DBInfo& info);
    ~DBConnection();

    DISALLOW_COPY_AND_ASSIGN(DBConnection);

    bool execute(const char* sql);
    QueryResult query(const char* sql);

    std::string getError(); // 得到最后一次错误

private:
    bool init();

    st_mysql* conn_; // mysql连接
    DBInfo info_;
};


} // namespace net

#endif // NET_DB_CONNECTION_H
