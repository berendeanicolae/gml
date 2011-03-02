#ifndef __BATCH_PERCEPTRON__
#define __BATCH_PERCEPTRON__

#include "GenericPerceptron.h"

class BatchPerceptron: public GenericPerceptron
{
	bool	PerformTrainIteration();
	bool	PerformTestIteration();
	bool	OnUpdateBestData();
	bool	OnSaveData(char *fileName);	
	void	OnRunThreadCommand(PerceptronThreadData &ptd,UInt32 command);
public:
	BatchPerceptron();
}; 

#endif
