#ifndef __CascadeFeatureSelection__
#define __CascadeFeatureSelection__

#include "GenericFeatureStatistics.h"

class CascadeFeatureSelection: public GenericFeatureStatistics
{
public:
	CascadeFeatureSelection();
	bool				OnProcessRecord(UInt32 recordIndex,GML::Algorithm::MLThreadData &thData);
	void				OnCompute();
};

#endif

