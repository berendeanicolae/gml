#ifndef __DB_Container__
#define __DB_Container__

#include "gmllib.h"

#define OP_DATA_SIZE	64


struct FeatInfo
{
	UInt64	Key;
	UInt32	ColumnIndex;
	UInt32	Type;
	UInt8	Op;
	UInt16	Params;
	UInt32	ParamsStartIndex;
};


class Container: public GML::DB::IDataBase
{
public:
	enum 
	{
		OP_NONE = 0,
		OP_BIGGER,
		OP_BIGGER_EQ,
		OP_SMALLER,
		OP_SMALLER_EQ,
		OP_EQ,
		OP_DIFF,
		OP_INSIDE,
		OP_OUTSIDE,
		OP_MASK,
		OP_MASK_EQ,
		OP_IN,
		OP_OUT,
	};
protected:
	double								Label;
	GML::Utils::GString					FeatureInformationFile;
	GML::Utils::GTFVector<FeatInfo>		FInfo;
	GML::Utils::GTFVector<UInt32>		ParamValues;
	bool								AddExtraInfoToFeatureName;

	bool								ProcessExpression(GML::Utils::GString &expre,FeatInfo &fi);
	bool								LoadFIF();
public:
	Container();

	bool								OnInit();
	bool								Close();
	bool								BeginIteration();
	bool								OnReadNextRecord(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr);
};
 

#endif
