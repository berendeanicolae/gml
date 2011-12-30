#ifndef __ONESIDE_ADJUST_PERCEPTRON__
#define __ONESIDE_ADJUST_PERCEPTRON__

#include "GenericPerceptron.h"

struct OneSideAdjustPerceptronData
{
	PerceptronVector	pv;
	double				mostDistant;
};
class OneSideAdjustPerceptron: public GenericPerceptron
{
	enum {
		MARGIN_POZITIVE = 0,
		MARGIN_NEGATIVE
	};
	enum {
		ADJUST_AFTER_EACH_ITERATION = 0,
		ADJUST_WHEN_ALGORITHM_ENDS
	};
	UInt32							MarginType;
	UInt32							AdjustMethod;
	double							MostDistantAdjustament;
	
	bool 	AdjustBias(OneSideAdjustPerceptronData &osapd,GML::Utils::Interval &range,GML::ML::MLRecord &rec,GML::Utils::Indexes *indexes);
	bool	OnInitThreadData(GML::Algorithm::MLThreadData &thData);
	bool	PerformTrainIteration(UInt32 iteration);
	bool	PerformTestIteration(GML::Utils::AlgorithmResult &Result);
	void	OnRunThreadCommand(GML::Algorithm::MLThreadData &td,UInt32 command);
public:
	OneSideAdjustPerceptron();
}; 

#endif
