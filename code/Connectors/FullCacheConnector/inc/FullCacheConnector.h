#ifndef _FULL_CACHE_CONNECTOR_H
#define _FULL_CACHE_CONNECTOR_H

#include "gmllib.h"

using namespace GML::ML;
using namespace GML::Utils;
using namespace GML::DB;

#define MAIN_TABLE_NAME "RecordsTable"

class FullCacheConnector: public GML::ML::IConector
{
private:
	UInt32	IntervalStart, IntervalEnd;
	UInt32	RecordsCount, FeaturesCount;
	
	GML::Utils::GTVector <MLRecord>	RecordsCache;

	BOOL	Initialized;


public:
	FullCacheConnector();
	~FullCacheConnector();

	virtual UInt32 GetRecordCount();

	virtual UInt32 GetFeatureCount();

	virtual bool GetRecord(MLRecord &record,UInt32 index );

	virtual bool CreateMlRecord( MLRecord &record );

	virtual bool SetRecordInterval( UInt32 start, UInt32 end );

	virtual bool OnInit();

	virtual bool FreeMLRecord( MLRecord &record );

	virtual bool Close();

	virtual UInt32 GetTotalRecordCount();

};

#endif