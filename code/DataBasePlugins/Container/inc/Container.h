#ifndef __DB_Container__
#define __DB_Container__

#include "gmllib.h"

class Container: public GML::DB::IDataBase
{
	
public:
	Container();

	bool				OnInit();
	bool				Close();
	bool				BeginIteration();
	bool				ReadNextRecord(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr);
};
 

#endif
