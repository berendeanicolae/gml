#ifndef __Rosenblatt__Perceptron__
#define __Rosenblatt__Perceptron__

#include "GenericPerceptron.h"

class RosenblattPerceptron: public GenericPerceptron
{
	bool	PerformTrainIteration();
	bool	PerformTestIteration();
	bool	OnUpdateBestData();
	bool	OnSaveData(char *fileName);	
public:
	RosenblattPerceptron();
};


#endif

