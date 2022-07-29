
#include <iostream>
#include "net_ext/db/db_thread_manager.h"
using namespace net;
using namespace std;


int main()
{
    DBInfo info;
    info.userName = "guest";
    info.password = "guest";
    info.dbName = "information_schema";

    DBThreadManager dbManager(info);
    dbManager.setThreadNum(4);
    dbManager.start();
    dbManager.addTask([](DBConnection * db)
    {
        QueryResult res = db->query("SELECT * FROM ENGINES");
        if (res)
        {
            cout << "rows: " << res.rows() << endl;
            cout << "fields: " << res.fields() << endl;
            while (char** row = res.fetchRow())
            {
                cout << row[0] << " " << row[1] << endl;
            }
        }
    });
}
