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
	};

private:
	UInt32					FeatureCount,RecordCount;
	GML::Utils::Indexes		Indexes;

	//properties
	UInt32					SplitMode;
	UInt32					Start,End;
	UInt32					attrStart, attrEnd;

	bool					CreateIndexList();
	bool					AddIndexes();
	bool					CreateUniformPercentageIndex();
	
public:
	SplitConnector();
	~SplitConnector();

	UInt32					GetRecordCount();
	bool					GetRecordLabel( double &label,UInt32 index );
	UInt32					GetFeatureCount();	
	bool					GetRecord( MLRecord &record,UInt32 index );
	bool					CreateMlRecord( MLRecord &record );
	bool					SetRecordInterval( UInt32 start, UInt32 end );
	bool					OnInit();
	bool					FreeMLRecord( MLRecord &record );
	bool					Close();
	UInt32					GetTotalRecordCount();	
	bool					OnInitPercentage();
};

