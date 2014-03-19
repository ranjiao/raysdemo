#ifndef DBOPERATOR_H
#define DBOPERATOR_H

#include <string.h>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <vector>
#include <wx/wx.h>

using namespace std;

struct Song
{
    wxString name;
    wxString country;
    wxString gender;
    wxString band;
    wxString language;
    wxString singer;
};

typedef vector<Song>            Songs;
typedef vector<Song>::iterator  SongsIter;

class DBOperator
{
public:
    DBOperator(void);
    virtual ~DBOperator(void);

    bool IsConnected();
    bool DBConnect(string user, string passwd, string url, string schema);
    void DBDisconnect();
    bool DBExecute(std::string stat);

    bool SearchSong(string stat, Songs& result);
protected:
    sql::mysql::MySQL_Driver *driver;
    sql::Connection *con;
    bool m_connected;
};

#endif