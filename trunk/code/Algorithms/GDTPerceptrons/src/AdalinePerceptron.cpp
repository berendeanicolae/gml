#include "AdalinePerceptron.h"

AdalinePerceptron::AdalinePerceptron()
{
	ObjectName = "AdalinePerceptron";
	RemoveProperty("ThreadsCount");
	RemoveProperty("AdjustWeightMode");
	
	adjustWeightMode = ADJUST_WEIGHT_LEASTMEANSQUARE;
}

bool AdalinePerceptron::PerformTrainIteration()
{
	return Train(&FullData,false,false);
}
bool AdalinePerceptron::PerformTestIteration()
{
	return Test(&FullData);
}
