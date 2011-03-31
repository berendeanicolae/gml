#ifndef __DB_MYSQL_H
#define __DB_MYSQL_H

#include "gmllib.h"
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
	GML::Utils::GString		Server, Database, Username, Password;
	UInt32					Port;
};

class DB_MySQL: public GML::DB::IDataBase
{
private:
	DB_CONN_STR DBConnStr;
	MYSQL *conn;
	DB_RES_BUFF DBResBuff;

	UInt32 GetRowCount(char *Statement);
	bool CheckCursorPos(char *Statement="");
	bool SetDataType();
	bool NotifyError(char* Msg="");
	bool StripQ(char* Query, const char* Word);
	bool FillRow(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr, MYSQL_ROW Row);
public:
	DB_MySQL();
	~DB_MySQL();
	
	bool OnInit();
	bool Connect ();
	bool Disconnect ();
	UInt32 Select (char* Statement="*");
	UInt32 SqlSelect (char* What="*", char* Where="", char* From="");	
	bool FetchNextRow (GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr);
	bool FetchRowNr (GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr, UInt32 RowNr);
	bool FreeRow(GML::Utils::GTFVector<GML::DB::DBRecord> &Vect);
	bool GetColumnInformations(char *columnName,GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr);
	bool InsertRow (char* Table, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect);
	bool InsertRow (char* Table, char* Fields, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect);
	bool Update (char* SqlStatement, GML::Utils::GTFVector<GML::DB::DBRecord> &WhereVals, GML::Utils::GTFVector<GML::DB::DBRecord> &UpdateVals);
};

#endif