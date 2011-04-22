#ifndef __BATCH_PERCEPTRON__
#define __BATCH_PERCEPTRON__

#include "GenericPerceptron.h"

struct MarginThreadData
{
	PerceptronVector		Delta;
	GML::Utils::Interval	MarginRange;
	GML::Utils::Interval	OriginalMarginRange;
};
class MarginPerceptron: public GenericPerceptron
{
	enum {
		MARGIN_POZITIVE = 0,
		MARGIN_NEGATIVE
	};
	enum {
		MARGIN_BATCH = 0,
		MARGIN_STREAM
	};

	UInt32							MarginType;
	UInt32							MaxError;
	UInt32							MarginTrainMethod;
	GML::Utils::Indexes				MarginIndexes,WorkMarginIndexes;

protected:

	bool	OnInitThreadData(GML::Algorithm::MLThreadData &thData);
	void	OnRunThreadCommand(GML::Algorithm::MLThreadData &td,UInt32 command);
	bool	PerformTrainIterationForBatchData(UInt32 iteration);
	bool	PerformTrainIterationForStreamData(UInt32 iteration);
	bool	PerformTrainIteration(UInt32 iteration);
	bool	PerformTestIteration(GML::Utils::AlgorithmResult &Result);


	bool	OnInit();
	bool	TestAndReduce(PerceptronVector &pv,GML::ML::MLRecord &Record,GML::Utils::Indexes &indexes,GML::Utils::Interval &Range);
public:
	MarginPerceptron();
}; 

#endif
