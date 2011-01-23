#ifndef __DB_MYSQL_H
#define __DB_MYSQL_H

#include "gml.h"
#include <mysql.h>
#include <stdio.h>

struct DB_RES_BUFF
{
	MYSQL_RES *res;
	char *QueryStr;
	char **ColumnNames;
	UInt32 *ColumnTypes;
	UInt32 pos_curr, pos_max;
	UInt32 lim_inf, window_size;
	UInt32 field_num; 
};

struct DB_CONN_STR 
{
	char *Server, *Database, *Username, *Password;
	UInt32 Port;
};

class DB_MySQL: public IDatabase 
{
private:
	INotifier *notifier;
	DB_CONN_STR DBConnStr;
	MYSQL *conn;
	DB_RES_BUFF DBResBuff;

	UInt32 GetRowCount(char *Statement);
	bool CheckCursorPos(char *Statement="");
	bool SetDataType();
	void NotifyError(char* Msg="");
	bool StripQ(char* Query, const char* Word);
	bool FillRow(DbRecordVect &VectPtr, MYSQL_ROW Row);
public:
	DB_MySQL();
	~DB_MySQL();
	
	bool Init (INotifier &notifier, char* Server, char* Database="", char* Username="", char* Password="", UInt32 Port=3306);
	bool Connect ();
	bool Disconnect ();
	UInt32 Select (char* Statement="*");
	UInt32 SqlSelect (char* What="*", char* Where="", char* From="");	
	bool FetchNextRow (DbRecordVect &VectPtr);
	bool FetchRowNr (DbRecordVect &VectPtr, UInt32 RowNr);
	bool FreeRow(DbRecordVect &Vect);
	bool InsertRow (char* Table, DbRecordVect &Vect);
	bool InsertRow (char* Table, char* Fields, DbRecordVect &Vect);
	bool Update (char* SqlStatement, DbRecordVect &WhereVals, DbRecordVect &UpdateVals);
};

#endif