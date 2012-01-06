#ifndef __Decision_Tree_Feature_Statistics__
#define __Decision_Tree_Feature_Statistics__

#include "GenericFeatureStatistics.h"

class DecisionTreeFeatureStatistics: public GenericFeatureStatistics
{
	GML::Utils::GTFVector<UInt32>			BTree;
	GML::Utils::GTFVector<UInt64>			HashesForRecord;
	UInt32									MaxDepth;
	bool									CreatePath(UInt32 index);
public:
	DecisionTreeFeatureStatistics();

	bool				Init();
	void				OnCompute();
	bool				OnThreadComputeExtraData(UInt32 recordIndex,GML::Algorithm::MLThreadData &thData);
	bool				WriteData(GML::Utils::File& out);
	bool				WritePaths(GML::Utils::File& out);
	bool				WriteFlags(GML::Utils::File& out);
};

#endif

