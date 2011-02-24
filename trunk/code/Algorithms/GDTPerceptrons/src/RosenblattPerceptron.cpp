#include "RosenblattPerceptron.h"

bool RosenblattPerceptron::PerformTrainIteration()
{
	return Train(&FullData);
}
bool RosenblattPerceptron::PerformTestIteration()
{
	if (Test(&FullData)==false)
		return false;
	notif->Notify(100,&FullData.Res,sizeof(FullData.Res));
	return true;
}
