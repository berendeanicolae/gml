#ifndef __BATCH_PERCEPTRON__
#define __BATCH_PERCEPTRON__

#include "GenericPerceptron.h"

class OneSidePerceptron: public GenericPerceptron
{
	bool	PerformTrainIteration();
	bool	PerformTestIteration();
	void	OnRunThreadCommand(PerceptronThreadData &ptd,UInt32 command);
public:
	OneSidePerceptron();
}; 

#endif
