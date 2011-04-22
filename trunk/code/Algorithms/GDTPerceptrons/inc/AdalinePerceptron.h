#ifndef __ADALINE_PERCEPTRON__
#define __ADALINE_PERCEPTRON__

#include "GenericPerceptron.h"

class AdalinePerceptron : public GenericPerceptron
{
	GML::Utils::Interval	range;

	bool	PerformTrainIteration(UInt32 iteration);
	bool	PerformTestIteration(GML::Utils::AlgorithmResult &Result);
	bool	OnInit();
public:
	AdalinePerceptron();
};


#endif

