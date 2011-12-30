#include "OneSideAdjustPerceptron.h"

#define COMMAND_TRAIN		0
#define COMMAND_TEST		1
#define COMMAND_ADJUST		2


OneSideAdjustPerceptron::OneSideAdjustPerceptron()
{
	ObjectName = "OneSideAdjustPerceptron";
	
	LinkPropertyToUInt32("MarginType"				,MarginType				,MARGIN_NEGATIVE,"!!LIST:Positive=0,Negative!!");	
	LinkPropertyToDouble("MostDistantAdjustament"	,MostDistantAdjustament	,0.000001		,"Value to adjust Bias for the most distant record");
	LinkPropertyToUInt32("AdjustMethod"				,AdjustMethod, ADJUST_AFTER_EACH_ITERATION,"!!LIST:AfterEachIteration=0,WhenAlgorithmEnds!!");	
}
bool OneSideAdjustPerceptron::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	OneSideAdjustPerceptronData	*osapd = new OneSideAdjustPerceptronData();
	if (osapd==NULL)
		return false;
	if (osapd->pv.Create(con->GetFeatureCount())==false)
		return false;
	thData.Context = osapd;
	return true;
}
void OneSideAdjustPerceptron::OnRunThreadCommand(GML::Algorithm::MLThreadData &td,UInt32 command)
{
	OneSideAdjustPerceptronData *osapd = (OneSideAdjustPerceptronData *)td.Context;
	switch (command)
	{
		case COMMAND_TRAIN:
			Train(osapd->pv,pvMain,td.Range,td.Record,&RecordIndexes);
			break;
		case COMMAND_TEST:
			Test(pvMain,td.Range,td.Record,td.Res,&RecordIndexes,true,false);
			break;
		case COMMAND_ADJUST:
			AdjustBias(*osapd,td.Range,td.Record,&RecordIndexes);
			break;			
	}
}
bool OneSideAdjustPerceptron::AdjustBias(OneSideAdjustPerceptronData &osapd,GML::Utils::Interval &range,GML::ML::MLRecord &rec,GML::Utils::Indexes *indexes)
{
	UInt32	*ptrIndex;
	UInt32	currentIndex,count;
	UInt32	nrFeatures = con->GetFeatureCount();
	double	sum;

	count = range.Size();
	if (indexes==NULL)
	{
		currentIndex = range.Start;
	} else {
		if (range.End>indexes->Len())
		{
			notif->Error("[%s] -> Invalid Range (%d..%d) for %d records",ObjectName,range.Start,range.End,indexes->Len());
			return false;
		}
		ptrIndex = indexes->GetList();
		ptrIndex += range.Start;
	}
	// use or not Bias
	if (!useB)
	{
		notif->Error("[%s] -> Bias is required for OnSide Adjustament",ObjectName);
		return false;
	}
	// train efectiv
	while (count>0)
	{
		if (indexes!=NULL)
			currentIndex = (*ptrIndex);
		// citesc elementul
		if (con->GetRecord(rec,currentIndex)==false)
		{
			notif->Error("[%s] -> Error reading record #%d",ObjectName,currentIndex);
			return false;
		}
		sum = GML::ML::VectorOp::ComputeVectorsSum(rec.Features,pvMain.Weight,nrFeatures)+(pvMain.Bias);
		
		if (sum * rec.Label<=0)
		{
			if ((MarginType == MARGIN_POZITIVE) && (rec.Label>0))
				osapd.mostDistant = min(sum,osapd.mostDistant);
			else if ((MarginType == MARGIN_NEGATIVE) && (rec.Label<0))				
				osapd.mostDistant = max(sum,osapd.mostDistant);
		}
		// next element
		if (indexes!=NULL)
			ptrIndex++;
		else
			currentIndex++;
		count--;
	}
	//notif->Error("[%s] -> Most distant: %lf",ObjectName,osapd.mostDistant);
	return true;
}
bool OneSideAdjustPerceptron::PerformTrainIteration(UInt32 iteration)
{
	UInt32							tr;
	
	// curat datele -> delta = 0
	for (tr=0;tr<threadsCount;tr++)
		((OneSideAdjustPerceptronData *)ThData[tr].Context)->pv.ResetValues();
	
		
	ExecuteParalelCommand(COMMAND_TRAIN);
		
	for (tr=0;tr<threadsCount;tr++)
		pvMain.Add(((OneSideAdjustPerceptronData *)ThData[tr].Context)->pv);
	
	return true;
}
bool OneSideAdjustPerceptron::PerformTestIteration(GML::Utils::AlgorithmResult &Result)
{
	bool doAdjust = false;
	// curat valoarea de bias
	if (AdjustMethod == ADJUST_AFTER_EACH_ITERATION)
		doAdjust = true;
	if ((AdjustMethod == ADJUST_WHEN_ALGORITHM_ENDS) && (Result.Iteration+1>=maxIterations))
	{
		//notif->Info("[%s] -> Final iterations (adjusting Bias for OneSide values)",ObjectName);
		doAdjust = true;
	}
		
	if (doAdjust)
	{
		for (UInt32 tr=0;tr<threadsCount;tr++)
			((OneSideAdjustPerceptronData *)ThData[tr].Context)->mostDistant=0.0;
		ExecuteParalelCommand(COMMAND_ADJUST);
		// ajustez bias-ul
		double newBias = 0.0;
		if (MarginType == MARGIN_POZITIVE)
		{
			for (UInt32 tr=0;tr<threadsCount;tr++)
				newBias = min(((OneSideAdjustPerceptronData *)ThData[tr].Context)->mostDistant,newBias);
			newBias -= MostDistantAdjustament;
		}
		if (MarginType == MARGIN_NEGATIVE)
		{
			for (UInt32 tr=0;tr<threadsCount;tr++)
				newBias = max(((OneSideAdjustPerceptronData *)ThData[tr].Context)->mostDistant,newBias);
			newBias += MostDistantAdjustament;
		}
		//notif->Info("[%s] -> CurentBias: %lf, Max found:%lf",ObjectName,pvMain.Bias,newBias);
		pvMain.Bias = pvMain.Bias-newBias;
	}
	
	ExecuteParalelCommand(COMMAND_TEST);		
	for (UInt32 tr=0;tr<threadsCount;tr++)
		Result.Add(&ThData[tr].Res);
	//Result.Compute(); // calculeaza apelantul

	return true;
}
