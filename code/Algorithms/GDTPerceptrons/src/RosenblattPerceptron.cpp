#include "RosenblattPerceptron.h"

RosenblattPerceptron::RosenblattPerceptron()
{
	ObjectName = "RosenblattPerceptron";
	RemoveProperty("ThreadsCount");
}
bool RosenblattPerceptron::OnInit()
{
	range.Set(0,RecordIndexes.Len());
	return true;
}
bool RosenblattPerceptron::PerformTrainIteration(UInt32 iteration)
{
	return Train(pvMain,pvMain,range,MainRecord,&RecordIndexes);
}
bool RosenblattPerceptron::PerformTestIteration(GML::Utils::AlgorithmResult &Result)
{
	return Test(pvMain,range,MainRecord,Result,&RecordIndexes);
}
