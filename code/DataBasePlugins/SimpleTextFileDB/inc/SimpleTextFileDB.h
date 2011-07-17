#ifndef __SIMPLE_TEXT_FILE_DB__
#define __SIMPLE_TEXT_FILE_DB__

#include "gmllib.h"

class SimpleTextFileDB: public GML::DB::IDataBase
{
	GML::Utils::GString		line,token;
public:
	SimpleTextFileDB();

	bool				OnInit();
	bool				Close();
	bool				BeginIteration();
	bool				OnReadNextRecord(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr);
};
 

#endif
