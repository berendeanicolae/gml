#ifndef __BATCH_PERCEPTRON__
#define __BATCH_PERCEPTRON__

#include "GenericPerceptron.h"

class MarginPerceptron: public GenericPerceptron
{
	enum {
		MARGIN_POZITIVE = 0,
		MARGIN_NEGATIVE
	};

	UInt32							MarginType;
	UInt32							MaxError;
	GML::Utils::Indexes				MarginIndexes,WorkMarginIndexes;

protected:
	bool	PerformTrainIteration();
	bool	PerformTestIteration();
	void	OnRunThreadCommand(PerceptronThreadData &ptd,UInt32 command);
	bool	OnInit();
	bool	TestAndReduce(GML::Utils::Indexes *indexes,PerceptronThreadData *ptd);
public:
	MarginPerceptron();
}; 

#endif
