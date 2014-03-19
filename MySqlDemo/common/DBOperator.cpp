#include "DBOperator.h"
#include <assert.h>

using namespace std;

wxString buff;
#define UTF2STR(str) wxString(str, wxCSConv(wxFONTENCODING_UTF8))
#define STR2UTF(var) \
    buff = wxString(var.c_str(), wxCSConv(wxFONTENCODING_GB2312)); \
    var = buff.ToUTF8();

DBOperator::DBOperator(void):
driver(NULL), con(NULL), m_connected(false)
{
}

DBOperator::~DBOperator(void)
{
    if(con) delete con; con = NULL;
}

bool DBOperator::DBConnect(string user, string passwd, string url, string schema)
{
    if(con)
        DBDisconnect();

    try 
    {
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(url, user, passwd);
        con->setSchema(schema);
    } catch (sql::SQLException &e) {
        /*
          The MySQL Connector/C++ throws three different exceptions:
	
          - sql::MethodNotImplementedException (derived from sql::SQLException)
          - sql::InvalidArgumentException (derived from sql::SQLException)
          - sql::SQLException (derived from std::runtime_error)
        */
        cout << "# ERR: SQLException in " << __FILE__;
        cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
        /* Use what() (derived from std::runtime_error) to fetch the error message */
        cout << "# ERR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << " )" << endl;
        m_connected = false;
        return false;
    }
    m_connected = true;
    return true;
}

void DBOperator::DBDisconnect()
{
    if(con) delete con; con = NULL;
    m_connected = false;
}

bool DBOperator::DBExecute(std::string stat)
{
    sql::Statement *stmt;
    //bool flag = false;

    if(!IsConnected())
        return false;

    STR2UTF(stat);
    try 
    {
        stmt = con->createStatement();
        //flag = stmt->execute(stat);
        stmt->execute(stat);
    }
    catch (sql::SQLException &e) 
    {
        delete stmt;
        return false;
    }

    delete stmt;
    return true;
}

bool DBOperator::IsConnected()
{
    return m_connected;
}

bool DBOperator::SearchSong(string stat, Songs& result)
{
    sql::Statement *stmt = NULL;
    sql::ResultSet *res = NULL;

    assert(con);
    STR2UTF(stat);
    try 
    {
        stmt = con->createStatement();
        res = stmt->executeQuery(stat);
        while (res->next()) 
        {
            Song newSong;
            newSong.name = UTF2STR(res->getString("name").c_str());
            newSong.country =  wxString(res->getString("country").c_str(),wxCSConv(wxFONTENCODING_UTF8));
            newSong.gender =  wxString(res->getString("gender").c_str(),wxCSConv(wxFONTENCODING_UTF8));
            newSong.band =  wxString(res->getString("band").c_str(),wxCSConv(wxFONTENCODING_UTF8));
            newSong.language = wxString(res->getString("language").c_str(),wxCSConv(wxFONTENCODING_UTF8));
            newSong.singer = wxString(res->getString("singer").c_str(),wxCSConv(wxFONTENCODING_UTF8));

            result.push_back(newSong);
        }
    } 
    catch (sql::SQLException &e) 
    {
        if(res) delete res; res = NULL;
        if(stmt) delete stmt; stmt = NULL;
        return false;
    }

    delete res;
    delete stmt;
    return true;
}