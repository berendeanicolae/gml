#ifndef __BATCH_PERCEPTRON__
#define __BATCH_PERCEPTRON__

#include "GenericPerceptron.h"

class OneSidePerceptron: public GenericPerceptron
{
	enum {
		MARGIN_POZITIVE = 0,
		MARGIN_NEGATIVE
	};

	UInt32							MarginType;
	GML::Utils::Indexes				MarginIndexes,WorkMarginIndexes,TrainIndexes;

protected:
	bool	PerformTrainIteration();
	bool	PerformTestIteration();
	void	OnRunThreadCommand(PerceptronThreadData &ptd,UInt32 command);
	bool	OnInit();
	bool	TestAndReduce(GML::Utils::Indexes *indexes,PerceptronThreadData *ptd);
public:
	OneSidePerceptron();

};

#endif
