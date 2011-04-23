#include "gmllib.h"

using namespace GML;
using namespace GML::DB;
using namespace GML::ML;
using namespace GML::Utils;




class ShuffleConnector : public IConnector
{
protected:

	enum {
		SHUFFLE_METHOD_POZITIVE_FIRST = 0,
		SHUFFLE_METHOD_NEGATIVE_FIRST,
		SHUFFLE_METHOD_RANDOM,
		SHUFFLE_METHOD_UNIFORM
	};
private:
	UInt32					FeatureCount,RecordCount;
	GML::Utils::Indexes		Indexes;

	//properties
	UInt32					Method;

	bool					ShufflePozitiveFirst();
	bool					ShuffleNegativeFirst();
	bool					OnInitConnectionToConnector();
public:
	ShuffleConnector();
	~ShuffleConnector();

	UInt32					GetRecordCount();
	bool					GetRecordLabel( double &label,UInt32 index );
	UInt32					GetFeatureCount();	
	bool					GetRecord( MLRecord &record,UInt32 index,UInt32 recordMask=0 );
	bool					GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index);
	bool					CreateMlRecord( MLRecord &record );
	bool					SetRecordInterval( UInt32 start, UInt32 end );
	bool					FreeMLRecord( MLRecord &record );
	bool					Close();
	UInt32					GetTotalRecordCount();	
	bool					OnInitPercentage();
	bool					GetFeatureName(GML::Utils::GString &str,UInt32 index);
};

