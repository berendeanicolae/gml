#include "gmllib.h"

using namespace GML;
using namespace GML::DB;
using namespace GML::ML;
using namespace GML::Utils;




class SplitConnector : public IConnector
{
protected:
	enum 
	{
		Percentage=0,
		Range,
		UniformPercentage,
		CustomPercentage,
	};

private:
	UInt32					FeatureCount,RecordCount;
	GML::Utils::Indexes		Indexes;

	//properties
	UInt32					SplitMode;
	UInt32					Start,End;
	UInt32					attrStart, attrEnd;
	UInt32					pozitiveProcStart,pozitiveProcEnd;
	UInt32					negativeProcStart,negativeProcEnd;

	bool					CreateIndexList();
	bool					AddIndexes();
	bool					CheckProcentInterval(UInt32	pStart,UInt32 pEnd);
	bool					CreateUniformPercentageIndex(UInt32 pozStart,UInt32 pozEnd,UInt32 negStart,UInt32 negEnd);
	
	bool					OnInitConnectionToConnector();
public:
	SplitConnector();
	~SplitConnector();

	UInt32					GetRecordCount();
	bool					GetRecordLabel( double &label,UInt32 index );
	UInt32					GetFeatureCount();	
	bool					GetRecord( MLRecord &record,UInt32 index );
	bool					CreateMlRecord( MLRecord &record );
	bool					SetRecordInterval( UInt32 start, UInt32 end );
	bool					FreeMLRecord( MLRecord &record );
	bool					Close();
	UInt32					GetTotalRecordCount();	
	bool					OnInitPercentage();
};
