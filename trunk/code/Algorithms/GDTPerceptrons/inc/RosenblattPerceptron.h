#ifndef __Rosenblatt__Perceptron__
#define __Rosenblatt__Perceptron__

#include "GenericPerceptron.h"

class RosenblattPerceptron: public GenericPerceptron
{
	GML::Utils::Interval	range;

	bool	PerformTrainIteration(UInt32 iteration);
	bool	PerformTestIteration(GML::Utils::AlgorithmResult &Result);
	bool	OnInit();
public:
	RosenblattPerceptron();
};


#endif