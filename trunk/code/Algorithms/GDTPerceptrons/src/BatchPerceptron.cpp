#include "BatchPerceptron.h"

#define COMMAND_TRAIN		0
#define COMMAND_TEST		1


BatchPerceptron::BatchPerceptron()
{
	ObjectName = "BatchPerceptron";
}
void BatchPerceptron::OnRunThreadCommand(PerceptronThreadData &ptd,UInt32 command)
{
	switch (command)
	{
		case COMMAND_TRAIN:
			Train(&ptd);
			break;
		case COMMAND_TEST:
			Test(&ptd);
			break;
	}
}
bool BatchPerceptron::PerformTrainIteration()
{
	UInt32	tr,count;
	if (threadsCount==1)
		return Train(&FullData);
	// paralel mode
	count = con->GetFeatureCount();
	for (tr=0;tr<threadsCount;tr++)
	{
		memcpy(ptData[tr].Weight,FullData.Weight,sizeof(double)*count);
		ptData[tr].b_Weight = FullData.b_Weight;
	}
	ExecuteParalelCommand(COMMAND_TRAIN);
	// aditie de date
	
	memset(FullData.Weight,0,sizeof(double)*count);
	for (tr=0;tr<threadsCount;tr++)
		GML::ML::VectorOp::AddVectors(FullData.Weight,ptData[tr].Weight,count);
	return true;
}
bool BatchPerceptron::PerformTestIteration()
{
	UInt32	tr,count;

	if (threadsCount==1)
	{
		if (Test(&FullData)==false)
			return false;
	} else {
		count = con->GetFeatureCount();
		for (tr=0;tr<threadsCount;tr++)
		{
			memcpy(ptData[tr].Weight,FullData.Weight,sizeof(double)*count);
			ptData[tr].b_Weight = FullData.b_Weight;
		}
		ExecuteParalelCommand(COMMAND_TEST);
		FullData.Res.Clear();
		for (tr=0;tr<threadsCount;tr++)
			FullData.Res.Add(&ptData[tr].Res);
		FullData.Res.Compute();
	}
	FullData.Res.time.Stop();
	notif->Notify(100,&FullData.Res,sizeof(FullData.Res));
	return true;
}
bool BatchPerceptron::OnUpdateBestData()
{
	return UpdateBest(FullData);
}
bool BatchPerceptron::OnSaveData(char *fileName)
{
	return Save(FullData,fileName);
}