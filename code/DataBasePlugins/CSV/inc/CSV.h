#ifndef __CSV__
#define __CSV__

#include "gmllib.h"


#define MAX_COLUMN_NAME		128
struct Column
{
	char	Name[MAX_COLUMN_NAME];
	UInt32	Type;
};
class CSV: public GML::DB::IDataBase
{
	GML::Utils::GString							fileName,tempStr,featureIndex,allDB;
	UInt32										nrRecords,nrFeatures;
	UInt32										cIndex;
	GML::Utils::GTVector<Column>				Columns;
	int											dbPoz;
	bool										modQueryCount;
public:
	CSV();

	bool					OnInit();
	bool					Connect ();
	bool					Disconnect ();
	bool					ExecuteQuery (char* Statement,UInt32 *rowsCount=NULL);
	bool					FetchNextRow (GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr);
	bool					GetColumnInformations(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr);

	bool				InsertRow (char* Table, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect);
	bool				InsertRow (char* Table, char* Fields, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect);
	bool				Update (char* SqlStatement, GML::Utils::GTFVector<GML::DB::DBRecord> &WhereVals, GML::Utils::GTFVector<GML::DB::DBRecord> &UpdateVals);

};
 

#endif
