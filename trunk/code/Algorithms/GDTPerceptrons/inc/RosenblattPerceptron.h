#ifndef __Rosenblatt__Perceptron__
#define __Rosenblatt__Perceptron__

#include "GenericPerceptron.h"

class RosenblattPerceptron: public GenericPerceptron
{
	bool	PerformTrainIteration();
	bool	PerformTestIteration();

public:
	RosenblattPerceptron();
};


#endif

