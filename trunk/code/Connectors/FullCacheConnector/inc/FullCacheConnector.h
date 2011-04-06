#ifndef _FULL_CACHE_CONNECTOR_H
#define _FULL_CACHE_CONNECTOR_H

#include "gmllib.h"
#include <stdio.h>

#define _CRT_SECURE_NO_DEPRECATE 
#define _CRT_SECURE_NO_WARNINGS

using namespace GML;
using namespace GML::DB;
using namespace GML::ML;
using namespace GML::Utils;
						

class FullCacheConnector: public IConnector
{
private:
	UInt32		nrRecords, FeatureCount;
	double		*Records;
	double		*Labels;
		

public:
	FullCacheConnector();
	~FullCacheConnector();

	UInt32		GetRecordCount();
	UInt32		GetFeatureCount();
	bool		GetRecord(MLRecord &record,UInt32 index );
	bool		CreateMlRecord( MLRecord &record );
	bool		SetRecordInterval( UInt32 start, UInt32 end );
	bool		OnInit();
	bool		FreeMLRecord( MLRecord &record );
	bool		Close();
	UInt32		GetTotalRecordCount();
	bool		GetRecordLabel(double &label,UInt32 index);

};

#endif