#ifndef __SQLITEDATABASE_H
#define __SQLITEDATABASE_H
#include "sqlite3.h"
#include "gmllib.h"
using namespace GML::DB;



class SqliteDatabase: public GML::DB::IDataBase
{
private:
	UInt32 currentRow;
	sqlite3 *database;
	const char* database_name;
	const char* tail;
	sqlite3_stmt *res;
	GML::Utils::INotify* notifier; 
	bool _InsertRow(char* table, GML::Utils::GTVector<GML::DB::DBRecord> &Vect, char* Fields = "");
	char* _CreateStatement(char*Statement, GML::Utils::GTVector<GML::DB::DBRecord> &vals, GML::Utils::GTVector<GML::DB::DBRecord> &uvals);
public:
	SqliteDatabase();
	~SqliteDatabase();
	UInt32 SqlSelect(char* What="*", char* Where="", char* From="" );
	UInt32 Select (char* Statement="*");
	bool Update(char* SqlStatement, GML::Utils::GTVector<GML::DB::DBRecord> &WhereVals, GML::Utils::GTVector<GML::DB::DBRecord> &UpdateVals );
	bool FreeRow( GML::Utils::GTVector<GML::DB::DBRecord> &Vect );
	bool Disconnect();
	bool Init(GML::Utils::INotify &notifier, char* connectionString);
	bool Connect();
	bool FetchNextRow( GML::Utils::GTVector<GML::DB::DBRecord> &VectPtr );
	bool FetchRowNr( GML::Utils::GTVector<GML::DB::DBRecord> &VectPtr, UInt32 RowNr );
	bool InsertRow(char* Table, char* Fields, GML::Utils::GTVector<GML::DB::DBRecord> &Vect );
	bool InsertRow (char* Table, GML::Utils::GTVector<GML::DB::DBRecord> &Vect);
	// used to free the prepared statement to prevent leakage
	bool Finalize();
	bool OnInit();
};



#endif
