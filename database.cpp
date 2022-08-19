#include "DataBase.h"
#include<stdio.h>
#include<string>
DataBase::DataBase(){
    GetIdNum();
}
int DataBase::GetIdNum(){
        if(mysql_init(&conn)==NULL){
        printf("mysql_init():%s\n",mysql_error(&conn));
        return -1;
    }
    if(mysql_real_connect(&conn,"localhost","","","Users",0,NULL,CLIENT_FOUND_ROWS)==NULL){
        printf("mysql_real_connect():%s\n",mysql_error(&conn));
        return -1;
    }
    printf("1.Connected MySQL successful!\n");
    query="select count(*) from user";
    MYSQL_RES *res=NULL;
    MYSQL_ROW row;
    int rc,fields;
    rc=mysql_real_query(&conn,query.c_str(),query.size());
    if(rc!=0){
        printf("mysql_real_query():%s\n",mysql_error(&conn));
        return -1;
    }
    res=mysql_store_result(&conn);
    if(res==NULL){
        printf("mysql_restore_result():%s\n",mysql_error(&conn));
        return -1;
    }
    row=mysql_fetch_row(res);
    num=atoi(row[0]);
    return 0;
}

int DataBase::CheckUser(std::string id,std::string password){
    MYSQL_RES *res;
    MYSQL_ROW row;
    query="select * from user where id=";
    query+=ID;
    query+=" and password=";
    query+=pwd;
    if(ID>num){
        printf("ID ERROR!\n");
        return -1;
    }
    int rc,rows,fields,i;
    rc=mysql_real_query(&conn,query.c_str(),query.size());
    if(rc!=0){
        printf("mysql_real_query():%s\n",mysql_error(&conn));
        return -1;
    }
    res=mysql_store_result(&conn);
    if(res==NULL){
        printf("mysql_restore_result():%s\n",mysql_error(&conn));
        return -1;
    }
    rows=mysql_num_rows(res);
    if(rows==0)return -1;
    return 0;
}
int DataBase::AddUser(std::string name,std::string password){
    int rc;
    num++;
    query="insert into user values(";
    query+=std::__cxx11::to_string(num);
    query+=",'";
    query+=name;
    query+="',";
    query+=password;
    query+=")";
    rc=mysql_real_query(&conn,query.c_str(),query.size());
    if(rc!=0){
        printf("mysql_real_query():%s\n",mysql_error(&conn));
        return -1;
    }
    return 0;
}
DataBase::~DataBase(){
    mysql_close(&conn);
}