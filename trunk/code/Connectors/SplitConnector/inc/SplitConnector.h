#ifndef __SPLIT_CONNECTOR_H

#include "gmllib.h"

using namespace GML;
using namespace GML::DB;
using namespace GML::ML;
using namespace GML::Utils;




class SplitConnector : public IConector
{
protected:
	enum 
	{
		Percentage=0,
		Range,
	};

private:
	UInt32 RecordCount, FeatureCount, TotalRecordCount;
	double	*RecordIndexCache;

	//properties
	UInt32 SplitMode;
	UInt32 Start, Stop;

public:
	virtual UInt32 GetRecordCount();
	virtual bool GetRecordLabel( double &label,UInt32 index );
	virtual UInt32 GetFeatureCount();	
	virtual bool GetRecord( MLRecord &record,UInt32 index );
	virtual bool CreateMlRecord( MLRecord &record );
	virtual bool SetRecordInterval( UInt32 start, UInt32 end );
	virtual bool OnInit();
	virtual bool FreeMLRecord( MLRecord &record );
	virtual bool Close();
	virtual UInt32 GetTotalRecordCount();	
	bool OnInitPercentage();
	bool OnInitRange();
	SplitConnector();
	~SplitConnector();
};

#endif