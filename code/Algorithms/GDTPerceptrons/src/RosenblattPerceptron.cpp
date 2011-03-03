#include "RosenblattPerceptron.h"

RosenblattPerceptron::RosenblattPerceptron()
{
	ObjectName = "RosenblattPerceptron";
}

bool RosenblattPerceptron::PerformTrainIteration()
{
	return Train(&FullData,false,false);
}
bool RosenblattPerceptron::PerformTestIteration()
{
	if (Test(&FullData)==false)
		return false;
	FullData.Res.time.Stop();
	notif->Notify(100,&FullData.Res,sizeof(FullData.Res));
	return true;
}
bool RosenblattPerceptron::OnUpdateBestData()
{
	return UpdateBest(FullData);
}
bool RosenblattPerceptron::OnSaveData(char *fileName)
{
	return Save(FullData,fileName);
}