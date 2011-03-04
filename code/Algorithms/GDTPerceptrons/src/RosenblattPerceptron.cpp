#include "RosenblattPerceptron.h"

RosenblattPerceptron::RosenblattPerceptron()
{
	ObjectName = "RosenblattPerceptron";
	RemoveProperty("ThreadsCount");
}

bool RosenblattPerceptron::PerformTrainIteration()
{
	return Train(&FullData,false,false);
}
bool RosenblattPerceptron::PerformTestIteration()
{
	return Test(&FullData);
}
