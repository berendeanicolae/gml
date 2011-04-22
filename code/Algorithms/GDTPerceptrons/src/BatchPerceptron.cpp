#include "BatchPerceptron.h"

#define COMMAND_TRAIN		0
#define COMMAND_TEST		1


BatchPerceptron::BatchPerceptron()
{
	ObjectName = "BatchPerceptron";
}
bool BatchPerceptron::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	PerceptronVector *pv;
	if ((pv = new PerceptronVector())==NULL)
		return false;
	if (pv->Create(con->GetFeatureCount())==false)
		return false;
	thData.Context = pv;
	return true;
}
void BatchPerceptron::OnRunThreadCommand(GML::Algorithm::MLThreadData &td,UInt32 command)
{
	switch (command)
	{
		case COMMAND_TRAIN:
			Train(*((PerceptronVector *)td.Context),pvMain,td.Range,td.Record,&RecordIndexes);
			break;
		case COMMAND_TEST:
			Test(pvMain,td.Range,td.Record,td.Res,&RecordIndexes,true,false);
			break;
	}
}
bool BatchPerceptron::PerformTrainIteration(UInt32 iteration)
{
	UInt32				tr;
	
	// curat datele -> delta = 0
	for (tr=0;tr<threadsCount;tr++)
		((PerceptronVector *)ThData[tr].Context)->ResetValues();
	
		
	ExecuteParalelCommand(COMMAND_TRAIN);
		
	for (tr=0;tr<threadsCount;tr++)
		pvMain.Add(*((PerceptronVector *)ThData[tr].Context));
	
	return true;
}
bool BatchPerceptron::PerformTestIteration(GML::Utils::AlgorithmResult &Result)
{
	//Result.Clear(); // a fost deja curatat de apelant
	ExecuteParalelCommand(COMMAND_TEST);		
	for (UInt32 tr=0;tr<threadsCount;tr++)
		Result.Add(&ThData[tr].Res);
	//Result.Compute(); // calculeaza apelantul

	return true;
}
