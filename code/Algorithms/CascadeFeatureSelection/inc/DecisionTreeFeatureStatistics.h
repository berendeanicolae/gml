#ifndef __Decision_Tree_Feature_Statistics__
#define __Decision_Tree_Feature_Statistics__

#include "GenericFeatureStatistics.h"

class DecisionTreeFeatureStatistics: public GenericFeatureStatistics
{
	GML::Utils::GTFVector<UInt32>			BTree;
	bool									CreatePath(UInt32 index);
public:
	DecisionTreeFeatureStatistics();

	bool				Init();
	void				OnCompute();
};

#endif

