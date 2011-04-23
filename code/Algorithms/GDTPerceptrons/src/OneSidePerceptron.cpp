#include "OneSidePerceptron.h"

#define COMMAND_TRAIN				0
#define COMMAND_TEST				1
#define COMMAND_TRAIN_ON_MARGIN		2
#define COMMAND_REDUCE				3

OneSidePerceptron::OneSidePerceptron()
{
	ObjectName = "OneSidePerceptron";

	LinkPropertyToUInt32("MarginType"			,MarginType			,MARGIN_NEGATIVE,"!!LIST:Positive=0,Negative!!");
	LinkPropertyToUInt32("MarginTrainMethod"	,MarginTrainMethod	,MARGIN_BATCH,"!!LIST:Batch=0,Stream!!");
}
bool OneSidePerceptron::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	MarginThreadData	*mtd;

	if ((mtd = new MarginThreadData())==NULL)
		return false;
	if (mtd->Delta.Create(con->GetFeatureCount())==false)
		return false;
	thData.Context = mtd;
	return true;
}
void OneSidePerceptron::OnRunThreadCommand(GML::Algorithm::MLThreadData &td,UInt32 command)
{
	MarginThreadData *mtd = (MarginThreadData*)td.Context;
	switch (command)
	{
		case COMMAND_TRAIN:
			Train(mtd->Delta,pvMain,td.Range,td.Record,&TrainIndexes);
			break;
		case COMMAND_TEST:
			Test(pvMain,td.Range,td.Record,td.Res,&TrainIndexes,true,false);
			break;
		case COMMAND_TRAIN_ON_MARGIN:
			Train(mtd->Delta,pvMain,mtd->MarginRange,td.Record,&WorkMarginIndexes);
			break;
		case COMMAND_REDUCE:
			TestAndReduce(pvMain,td.Record,WorkMarginIndexes,mtd->MarginRange);
			break;
	}
}
bool OneSidePerceptron::OnInit()
{
	UInt32					tr,count,splitValue,start;
	UInt32					*list;
	double					label;
	GML::Utils::Interval	tempRange;

	notif->Info("[%s] -> Analizing data ...",ObjectName);
	for (tr=0,count=0,list=RecordIndexes.GetList();tr<RecordIndexes.Len();tr++,list++)
	{
		if (con->GetRecordLabel(label,*list)==false)
		{
			notif->Error("[%s] -> Unable to read record #d label",ObjectName,(*list));
			return false;
		}
		if ((label==1.0) && (MarginType==MARGIN_POZITIVE))
			count++;
		if ((label==-1.0) && (MarginType==MARGIN_NEGATIVE))
			count++;
	}
	notif->Info("[%s] -> Found %d elements for requested margin ",ObjectName,count);
	if (MarginIndexes.Create(count)==false)
	{
		notif->Error("[%s] -> Unable to create MarginIndexes[%d]",ObjectName,count);
		return false;
	}
	if (WorkMarginIndexes.Create(count)==false)
	{
		notif->Error("[%s] -> Unable to create WorkMarginIndexes[%d]",ObjectName,count);
		return false;
	}
	if (TrainIndexes.Create(RecordIndexes.Len()-count)==false)
	{
		notif->Error("[%s] -> Unable to create TrainIndexes[%d]",ObjectName,RecordIndexes.Len()-count);
		return false;
	}
	// adaug si indexii
	for (tr=0,count,list=RecordIndexes.GetList();tr<RecordIndexes.Len();tr++,list++)
	{
		if (con->GetRecordLabel(label,*list)==false)
		{
			notif->Error("[%s] -> Unable to read record #d label",ObjectName,(*list));
			return false;
		}
		if (MarginType == MARGIN_POZITIVE)
		{
			if (label==1.0)
				MarginIndexes.Push(*list);
			else
				TrainIndexes.Push(*list);
		}
		if (MarginType == MARGIN_NEGATIVE)
		{
			if (label==-1.0)
				MarginIndexes.Push(*list);
			else
				TrainIndexes.Push(*list);
		}
	}
	if (MarginIndexes.Len()==0)
	{
		notif->Error("[%s] -> Invalid number of indexes for a margin (0). A margin must have at least one element !",ObjectName);
		return false;
	}
	if (TrainIndexes.Len()==0)
	{
		notif->Error("[%s] -> Invalid number of indexes for a train list (0). A margin must have at least one element !",ObjectName);
		return false;
	}
	notif->Info("[%s] -> Margin & Train indexes created (Train=%d , Margin=%d)",ObjectName,TrainIndexes.Len(),MarginIndexes.Len());
	// splituies pentru TrainIndexes
	if (SplitMLThreadDataRange(TrainIndexes.Len())==false)
		return false;

	splitValue = (MarginIndexes.Len()/threadsCount)+1;
	start = 0;
	for (tr=0;tr<threadsCount;tr++)
	{
		if ((start+splitValue)>=MarginIndexes.Len())
			splitValue = MarginIndexes.Len()-start;
		((MarginThreadData *)(ThData[tr].Context))->OriginalMarginRange.Set(start,start+splitValue);
		start+=splitValue;
	}


	return true;
}
bool OneSidePerceptron::TestAndReduce(PerceptronVector &pv,GML::ML::MLRecord &Record,GML::Utils::Indexes &indexes,GML::Utils::Interval &Range)
{
	UInt32	*ptrIndex;
	UInt32	*ptrLastNotTrained;
	UInt32	count = Range.Size();
	UInt32	nrFeatures = con->GetFeatureCount();
	UInt32	diff = 0;
	
	if (!useB)
		pv.Bias = 0.0;
	
	ptrIndex = indexes.GetList();
	ptrIndex+=Range.Start;
	ptrLastNotTrained = ptrIndex;

	while (count>0)
	{
		if (con->GetRecord(Record,*ptrIndex)==false)
		{
			notif->Error("[%s] -> Unable to read record #d",ObjectName,*ptrIndex);
			return false;
		}
		if (GML::ML::VectorOp::IsPerceptronTrained(Record.Features,pv.Weight,nrFeatures,pv.Bias,Record.Label)==true)
		{
			diff++;
			ptrIndex++;
		} else {
			(*ptrLastNotTrained) = (*ptrIndex);
			ptrLastNotTrained++;
			ptrIndex++;			
		}
		count--;
	}
	Range.End-=diff;

	return true;	
}

bool OneSidePerceptron::PerformTrainIterationForBatchData(UInt32 iteration)
{
	UInt32				tr,errorCount;
	MarginThreadData	*mtd;
	
	for (tr=0;tr<threadsCount;tr++)
		((MarginThreadData *)ThData[tr].Context)->Delta.ResetValues();
		
	ExecuteParalelCommand(COMMAND_TRAIN);
		
	for (tr=0;tr<threadsCount;tr++)
		pvMain.Add(((MarginThreadData *)ThData[tr].Context)->Delta);
	

	// refac indexii
	if (MarginIndexes.CopyTo(WorkMarginIndexes)==false)
	{
		notif->Error("[%s] -> Unable to copy indexes ...",ObjectName);
		return false;
	}
	// reducere efectiva
	for (tr=0;tr<threadsCount;tr++)
	{
		mtd = (MarginThreadData	*)ThData[tr].Context;
		mtd->MarginRange.Set(mtd->OriginalMarginRange.Start,mtd->OriginalMarginRange.End);		
	}
	do
	{
		for (tr=0;tr<threadsCount;tr++)
			((MarginThreadData *)ThData[tr].Context)->Delta.ResetValues();
		ExecuteParalelCommand(COMMAND_TRAIN_ON_MARGIN);
		for (tr=0;tr<threadsCount;tr++)
			pvMain.Add(((MarginThreadData *)ThData[tr].Context)->Delta);
		ExecuteParalelCommand(COMMAND_REDUCE);
		for (tr=0,errorCount=0;tr<threadsCount;tr++)
			errorCount+=((MarginThreadData *)ThData[tr].Context)->MarginRange.Size();
		//DEBUGMSG("[%s] -> ErrorCount = %d",errorCount);
	} while (errorCount>0);

	return true;
}
bool OneSidePerceptron::PerformTrainIterationForStreamData(UInt32 iteration)
{
	GML::Utils::Interval	Range;

	// antrenament normal
	Range.Set(0,TrainIndexes.Len());
	Train(pvMain,pvMain,Range,MainRecord,&TrainIndexes);

	// refac indexii
	if (MarginIndexes.CopyTo(WorkMarginIndexes)==false)
	{
		notif->Error("[%s] -> Unable to copy indexes ...",ObjectName);
		return false;
	}

	// reducere efectiva
	Range.Set(0,MarginIndexes.Len());
	do
	{
		Train(pvMain,pvMain,Range,MainRecord,&MarginIndexes);
		TestAndReduce(pvMain,MainRecord,MarginIndexes,Range);
		ExecuteParalelCommand(COMMAND_REDUCE);
	} while (Range.Size()>0);

	return true;
}
bool OneSidePerceptron::PerformTrainIteration(UInt32 iteration)
{
	switch (MarginTrainMethod)
	{
		case MARGIN_BATCH:
			return PerformTrainIterationForBatchData(iteration);
		case MARGIN_STREAM:
			return PerformTrainIterationForStreamData(iteration);
	}
	notif->Error("[%s] -> Unknown margin train method : %d",ObjectName,MarginTrainMethod);
	return false;
}

bool OneSidePerceptron::PerformTestIteration(GML::Utils::AlgorithmResult &Result)
{
	ExecuteParalelCommand(COMMAND_TEST);		
	for (UInt32 tr=0;tr<threadsCount;tr++)
		Result.Add(&ThData[tr].Res);
	if (MarginType==MARGIN_POZITIVE)
		Result.tp+=MarginIndexes.Len();
	if (MarginType==MARGIN_NEGATIVE)
		Result.tn+=MarginIndexes.Len();
	return true;
}
