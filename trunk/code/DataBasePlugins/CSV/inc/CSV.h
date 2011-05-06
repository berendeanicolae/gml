#ifndef __CSV__
#define __CSV__

#include "gmllib.h"

// formatul este :
// columnName1,columnName2,.....
// value1,value2,...
// value1,value2,....
//
// unde: columnName1 este de forma Nume{tip}, cu {tip optional}
//       {tip} = {double},{numeric},{int8},{int16},{int32},{uint8},...
//		 {tip} = i8,i16,i32,i64,u8,u16,u32,u64,b,f,d,h,n



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

	bool					UpdateValue(GML::Utils::GString &str,GML::DB::DBRecord &rec);
public:
	CSV();

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
