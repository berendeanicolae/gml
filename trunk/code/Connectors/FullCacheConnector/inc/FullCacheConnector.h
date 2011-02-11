#ifndef _FULL_CACHE_CONNECTOR_H
#define _FULL_CACHE_CONNECTOR_H

#include "gmllib.h"
#include <stdio.h>

#define _CRT_SECURE_NO_DEPRECATE 
#define _CRT_SECURE_NO_WARNINGS


using namespace GML::ML;
using namespace GML::Utils;
using namespace GML::DB;
							
#define RECORDS_TABLE_NAME			"RecordTable"
#define FEATURES_COL_PREFIX			"Feat"
#define HASH_COL_NAME				"Hash"
#define LABEL_COL_NAME				"Label"

#define MAX_SQL_QUERY_SIZE			2048

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