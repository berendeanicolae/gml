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
						

class FullCacheConnector: public IConector
{
private:
	UInt32	IntervalStart, IntervalEnd;
	UInt32	RecordCount, FeatureCount;
		
	double  * FeatureCache;
	double  * LabelStorage;

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

	virtual bool GetRecordLabel(double &label,UInt32 index);

};

#endif