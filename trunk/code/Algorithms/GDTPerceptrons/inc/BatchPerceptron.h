#ifndef __BATCH_PERCEPTRON__
#define __BATCH_PERCEPTRON__

#include "GenericPerceptron.h"

class BatchPerceptron: public GenericPerceptron
{
	bool	OnInitThreadData(GML::Algorithm::MLThreadData &thData);
	bool	PerformTrainIteration(UInt32 iteration);
	bool	PerformTestIteration(GML::Utils::AlgorithmResult &Result);
	void	OnRunThreadCommand(GML::Algorithm::MLThreadData &td,UInt32 command);
public:
	BatchPerceptron();
}; 

#endif
