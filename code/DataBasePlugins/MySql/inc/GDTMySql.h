#ifndef __GDT_MY_SQL__
#define __GDT_MY_SQL__

#include "gmllib.h"
#include <mysql.h>

class GDTMySQL: public GML::DB::IDataBase
{
	MYSQL					*conn;
	MYSQL_RES				*result;
	GML::Utils::GString		*columnNames;


	GML::Utils::GString		Server,Database,Username,Password;
	UInt32					Port;

	bool					MySQLError(char *text);	
public:
	GDTMySQL();
	bool					OnInit();
	bool					Connect ();
	bool					Disconnect ();
	UInt32					Select (char* Statement="*");
	UInt32					SqlSelect (char* What="*", char* Where="", char* From="");
	bool					FetchNextRow (GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr);
	bool					FetchRowNr (GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr, UInt32 RowNr);
	bool					FreeRow(GML::Utils::GTFVector<GML::DB::DBRecord> &Vect);
	bool					GetColumnInformations(char *columnName,GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr);
	bool					InsertRow (char* Table, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect);
	bool					InsertRow (char* Table, char* Fields, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect);
	bool					Update (char* SqlStatement, GML::Utils::GTFVector<GML::DB::DBRecord> &WhereVals, GML::Utils::GTFVector<GML::DB::DBRecord> &UpdateVals);
};

#endif