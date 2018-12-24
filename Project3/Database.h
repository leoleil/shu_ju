#pragma once
#include "Database.h"
#include <stdio.h>  
#include <mysql.h>  

#define _UNICODE

void sqlselect(MYSQL *, const char *);     //测试查询数据 
MYSQL *mysql = NULL;


int dataBase()
{
	//初始化MySQL连接句柄
	mysql = mysql_init((MYSQL *)0);

	mysql_real_connect
	(
		mysql,
		"localhost", //数据库地址
		"root", //数据库用户名
		"", //数据库密码
		"project", //数据库名称
		0, //数据库端口，0表示默认端口（即3306）
		NULL, //如果unix_socket不是NULL，字符串指定套接字或应该被使用的命名管道。注意host参数决定连接的类型
		0 //通常是0
	);

	if (!mysql) //连接失败
	{
		printf("Connection error:%d, %s\n", mysql_errno(mysql), mysql_error(mysql));
	}
	cout << "数据库连接成功" << endl;

	const char *command = "select * from commonnode"; //查询指令

												  // 改变编码格式
	mysql_set_character_set(mysql, "GB2312");

	sqlselect(mysql, command); //查询数据  

	mysql_close(mysql); //关闭连接  

	//system("pause");
	return 0;
}
void sqlselect(MYSQL *mysql, const char *command)
{

	int flag = mysql_real_query(mysql, command, strlen(command));

	if (flag)
	{
		printf("Select error:%d, %s\n", mysql_errno(mysql), mysql_error(mysql));
		return;
	}

	MYSQL_RES *res = mysql_store_result(mysql); //读取将查询结果   
	MYSQL_FIELD *field = mysql_fetch_fields(res); //获取所有列名
	int field_count = mysql_field_count(mysql); //获取列数

												//输出所有列名
	for (int i = 0; i < field_count; i++)
	{

		printf("%s\t", field[i].name);
	}

	printf("\n");

	//遍历输出每一行数据  
	MYSQL_ROW row;
	while (row = mysql_fetch_row(res))
	{
		for (int i = 0; i < field_count; i++)
		{
			printf("%s\t", row[i]);
		}
		printf("\n");
	}
}
