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

	GML::Utils::Indexes		Indexes;

	//properties
	UInt32					Method;

	bool					ShufflePozitiveFirst();
	bool					ShuffleNegativeFirst();
	bool					ShuffleRandom();
	bool					ShuffleUniform();
	bool					OnInitConnectionToConnector();
public:
	ShuffleConnector();
	~ShuffleConnector();

	bool					GetRecordLabel( double &label,UInt32 index );
	bool					GetRecord( MLRecord &record,UInt32 index,UInt32 recordMask=0 );
	bool					GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index);
	bool					CreateMlRecord( MLRecord &record );
	bool					FreeMLRecord( MLRecord &record );
	bool					Close();
	bool					OnInitPercentage();

};

