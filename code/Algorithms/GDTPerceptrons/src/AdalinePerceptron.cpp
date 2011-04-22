#include "AdalinePerceptron.h"

AdalinePerceptron::AdalinePerceptron()
{
	ObjectName = "AdalinePerceptron";
	RemoveProperty("ThreadsCount");
	RemoveProperty("AdjustWeightMode");
	
	adjustWeightMode = ADJUST_WEIGHT_LEASTMEANSQUARE;
}
bool AdalinePerceptron::OnInit()
{
	range.Set(0,RecordIndexes.Len());
	return true;
}
bool AdalinePerceptron::PerformTrainIteration(UInt32 iteration)
{
	return Train(pvMain,pvMain,range,MainRecord,&RecordIndexes);
}
bool AdalinePerceptron::PerformTestIteration(GML::Utils::AlgorithmResult &Result)
{
	return Test(pvMain,range,MainRecord,Result,&RecordIndexes);
}
