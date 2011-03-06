#ifndef __BATCH_PERCEPTRON__
#define __BATCH_PERCEPTRON__

#include "GenericPerceptron.h"

class OneSidePerceptron: public GenericPerceptron
{
	enum {
		ONESIDE_POZITIVE = 0,
		ONESIDE_NEGATIVE
	};

	UInt32							OneSideMargin;
	GML::Utils::Indexes				MarginIndexes;

protected:
	bool	PerformTrainIteration();
	bool	PerformTestIteration();
	void	OnRunThreadCommand(PerceptronThreadData &ptd,UInt32 command);
	bool	OnInit();
public:
	OneSidePerceptron();
}; 

#endif
