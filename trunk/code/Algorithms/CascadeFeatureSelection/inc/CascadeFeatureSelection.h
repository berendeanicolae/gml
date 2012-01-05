#ifndef __CascadeFeatureSelection__
#define __CascadeFeatureSelection__

#include "GenericFeatureStatistics.h"

class CascadeFeatureSelection: public GenericFeatureStatistics
{
public:
	CascadeFeatureSelection();
	
	void				OnCompute();
};

#endif

