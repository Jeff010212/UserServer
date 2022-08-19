#include<string>
#include<mysql.h>
#include<algorithm>
class DataBase{
private:
    MYSQL conn;
    std::string query;
    int num=0;
public:
    DataBase();
    ~DataBase();
    int GetIdNum();
    int CheckUser(std::string id,std::string password);
    int AddUser(std::string name,std::string password);
};