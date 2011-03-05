#include "OneSidePerceptron.h"

#define COMMAND_TRAIN		0
#define COMMAND_TEST		1


OneSidePerceptron::OneSidePerceptron()
{
	ObjectName = "OneSidePerceptron";
	batchPerceptron = true;
}
void OneSidePerceptron::OnRunThreadCommand(PerceptronThreadData &ptd,UInt32 command)
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
bool OneSidePerceptron::PerformTrainIteration()
{
	UInt32	tr;
	if (threadsCount==1)
	{
		return Train(&FullData,true,true);
	}
	
	
	// paralel mode
	ExecuteParalelCommand(COMMAND_TRAIN);
	// aditie de date
	
	for (tr=0;tr<threadsCount;tr++)
		FullData.Primary.Add(ptData[tr].Delta);		
	return true;
}
bool OneSidePerceptron::PerformTestIteration()
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
