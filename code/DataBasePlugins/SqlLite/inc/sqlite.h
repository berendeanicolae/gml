#ifndef __SQLITEDATABASE_H
#define __SQLITEDATABASE_H
#include "sqlite3.h"
#include "gmllib.h"


class SqliteDatabase: public GML::DB::IDataBase
{
private:
	UInt32 currentRow;
	sqlite3 *database;
	char* database_name;
	char* tail;
	sqlite3_stmt *res;
	GML::Utils::INotify* notifier; 
	bool _InsertRow(char* table, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect, char* Fields = "");
	char* _CreateStatement(char*Statement, GML::Utils::GTFVector<GML::DB::DBRecord> &vals, GML::Utils::GTFVector<GML::DB::DBRecord> &uvals);
public:
	int *Alloc() { return NULL; }
	SqliteDatabase();
	~SqliteDatabase();
	UInt32 SqlSelect(char* What="*", char* Where="", char* From="" );
	UInt32 Select (char* Statement="*");
	bool Update(char* SqlStatement, GML::Utils::GTFVector<GML::DB::DBRecord> &WhereVals, GML::Utils::GTFVector<GML::DB::DBRecord> &UpdateVals );
	bool FreeRow( GML::Utils::GTFVector<GML::DB::DBRecord> &Vect );
	bool Disconnect();
	bool Connect();
	bool FetchNextRow( GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr );
	bool FetchRowNr( GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr, UInt32 RowNr );
	bool InsertRow(char* Table, char* Fields, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect );
	bool InsertRow (char* Table, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect);
	// used to free the prepared statement to prevent leakage
	bool Finalize();
	bool OnInit();
};



#endif
