
#include "db_connection.h"
#include <string.h>
#include <mysql/mysql.h>
#include "util/logger.h"


namespace net
{

void QueryResult::free()
{
    if (res_) mysql_free_result(res_);
}

int QueryResult::rows() const
{
    return mysql_num_rows(res_);
}

int QueryResult::fields() const
{
    return mysql_num_fields(res_);
}

char** QueryResult::fetchRow()
{
    return mysql_fetch_row(res_);
}


DBConnection::DBConnection(const DBInfo& info)
    : info_(info)
{
    bool succeed = init();
    LOG_IF(FATAL, !succeed) << "fail to connect db: " << info.dbName;
}

DBConnection::~DBConnection()
{
    mysql_close(conn_);
}

// 初始化连接
bool DBConnection::init()
{
    // init the database connection
    conn_ = mysql_init(nullptr);

    /* connect the database */
    if (!mysql_real_connect(conn_, info_.hostName.c_str(), info_.userName.c_str(), info_.password.c_str(),
                            info_.dbName.c_str(), info_.dbPort, nullptr, 0))
    {
        return false;
    }

    // 是否连接已经可用
    if (mysql_query(conn_, "set names utf8")) return false;
    return true;
}

bool DBConnection::execute(const char* sql)
{
    if (mysql_query(conn_, sql)) return false;
    return true;
}

QueryResult DBConnection::query(const char* sql)
{
    if (mysql_real_query(conn_, sql, strlen(sql))) return nullptr;
    return mysql_store_result(conn_);
}

std::string DBConnection::getError()
{
    return mysql_error(conn_);
}

} // namespace net
