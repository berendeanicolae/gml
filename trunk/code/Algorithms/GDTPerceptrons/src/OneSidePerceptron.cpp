#include "OneSidePerceptron.h"

#define COMMAND_TRAIN		0
#define COMMAND_TEST		1


OneSidePerceptron::OneSidePerceptron()
{
	ObjectName = "OneSidePerceptron";
	batchPerceptron = true;

	LinkPropertyToUInt32("OneSideMargin"			,OneSideMargin			,ONESIDE_NEGATIVE,"!!LIST:Positive=0,Negative!!");
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
bool OneSidePerceptron::OnInit()
{
	UInt32	tr,count;
	UInt32	*list = RecordIndexes.GetList();
	double	label;

	for (tr=0,count;tr<RecordIndexes.Len();tr++,list++)
	{
		if (con->GetRecordLabel(label,*list)==false)
		{
			notif->Error("[%s] -> Unable to read record #d label",ObjectName,(*list));
			return false;
		}
		//if (
	}

	return true;
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
