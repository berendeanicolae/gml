#ifndef __PROJECTRON_H__
#define __PROJECTRON_H__

#include "GenericPerceptron.h"

class Projectron: public GenericPerceptron
{
	enum
	{
		POLYNOMIAL_KER = 0,
		SIGMOYD_KER
	};

	//The hilbert Space
	GML::Utils::Vector		Space;
	//K matrix (sort of a Gram matrix)
	GML::Utils::Matrix<double>	K;
	UInt32 kernelFunction;

public:
	bool	Train(PerceptronThreadData *ptd,bool clearDelta,bool addDeltaToPrimary);
	bool	Train(PerceptronThreadData *ptd,GML::Utils::Indexes *recordIndexes,bool clearDelta,bool addDeltaToPrimary);
	bool	PerformTrainIteration();
	bool	PerformTestIteration();
	bool	OnInit();

	UInt32 CountActiveFeatures(double *feat,UInt32 nrElements);

public:
	Projectron();
};

#endif