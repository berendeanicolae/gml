#ifndef __ADALINE_PERCEPTRON__
#define __ADALINE_PERCEPTRON__

#include "GenericPerceptron.h"

class AdalinePerceptron : public GenericPerceptron
{
	bool	PerformTrainIteration();
	bool	PerformTestIteration();

public:
	AdalinePerceptron();
};


#endif

