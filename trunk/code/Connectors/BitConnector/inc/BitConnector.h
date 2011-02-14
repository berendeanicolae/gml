#include "gmllib.h"

class BitConnector: public GML::ML::IConector
{
	UInt32	nrRecords,nrFeatures;
	UInt8	*Data;
	UInt32	*FeaturesIndexs;
	UInt32	LabelIndex,Align8Size;

	bool	UpdateColumnInfos(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr);
	
public:
	BitConnector();

	bool	OnInit();
	bool	Close();
	bool	SetRecordInterval(UInt32 start, UInt32 end);
	bool	CreateMlRecord (GML::ML::MLRecord &record);
	bool	GetRecord(GML::ML::MLRecord &record,UInt32 index);
	bool	FreeMLRecord(GML::ML::MLRecord &record);
	UInt32	GetFeatureCount();
	UInt32	GetRecordCount();
	UInt32	GetTotalRecordCount();
	
};
