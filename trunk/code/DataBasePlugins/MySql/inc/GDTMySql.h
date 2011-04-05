#ifndef __GDT_MY_SQL__
#define __GDT_MY_SQL__

#include "gmllib.h"
#include <mysql.h>

class GDTMySQL: public GML::DB::IDataBase
{
	MYSQL					*conn;
	MYSQL_RES				*result;
	

	GML::Utils::GString		Server,Database,Username,Password;
	UInt32					Port;

	bool					MySQLError(char *text);	
public:
	GDTMySQL();
	bool					OnInit();
	bool					Connect ();
	bool					Disconnect ();
	bool					ExecuteQuery (char* Statement,UInt32 *rowsCount=NULL);
	bool					FetchNextRow (GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr);
	bool					GetColumnInformations(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr);
	bool					InsertRow (char* Table, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect);
	bool					InsertRow (char* Table, char* Fields, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect);
	bool					Update (char* SqlStatement, GML::Utils::GTFVector<GML::DB::DBRecord> &WhereVals, GML::Utils::GTFVector<GML::DB::DBRecord> &UpdateVals);
};

#endif