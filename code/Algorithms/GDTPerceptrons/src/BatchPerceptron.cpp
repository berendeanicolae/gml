#include "BatchPerceptron.h"

#define COMMAND_TRAIN		0
#define COMMAND_TEST		1


BatchPerceptron::BatchPerceptron()
{
	ObjectName = "BatchPerceptron";
	batchPerceptron = true;
}
void BatchPerceptron::OnRunThreadCommand(PerceptronThreadData &ptd,UInt32 command)
{
	switch (command)
	{
		case COMMAND_TRAIN:
			Train(&ptd,true,false);
			break;
		case COMMAND_TEST:
			Test(&ptd);
			break;
	}
}
bool BatchPerceptron::PerformTrainIteration()
{
	UInt32	tr;
	//if (threadsCount==1)
	//{
	//	bool res = Train(&FullData,true,true);		
	//	return res;
	//}
	
	// paralel mode
	ExecuteParalelCommand(COMMAND_TRAIN);
	// aditie de date
	
	for (tr=0;tr<threadsCount;tr++)
		FullData.Primary.Add(ptData[tr].Delta);	
	
	return true;
}
bool BatchPerceptron::PerformTestIteration()
{
	UInt32	tr;

	if (threadsCount==1)
	{
		if (Test(&FullData)==false)
			return false;
	} else {
		ExecuteParalelCommand(COMMAND_TEST);
		FullData.Res.Clear();
		for (tr=0;tr<threadsCount;tr++)
			FullData.Res.Add(&ptData[tr].Res);
		FullData.Res.Compute();
	}

	return true;
}
