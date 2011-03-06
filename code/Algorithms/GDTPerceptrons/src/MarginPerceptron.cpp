#include "MarginPerceptron.h"

#define COMMAND_TRAIN				0
#define COMMAND_TEST				1
#define COMMAND_TRAIN_ON_MARGIN		2
#define COMMAND_REDUCE				3


MarginPerceptron::MarginPerceptron()
{
	ObjectName = "MarginPerceptron";
	batchPerceptron = true;

	LinkPropertyToUInt32("MarginType"			,MarginType			,MARGIN_NEGATIVE,"!!LIST:Positive=0,Negative!!");
	LinkPropertyToUInt32("MaxError"				,MaxError			,0);
}
void MarginPerceptron::OnRunThreadCommand(PerceptronThreadData &ptd,UInt32 command)
{
	switch (command)
	{
		case COMMAND_TRAIN:
			Train(&ptd,true,false);
			break;
		case COMMAND_TEST:
			Test(&ptd);
			break;
		case COMMAND_TRAIN_ON_MARGIN:
			Train(&ptd,&WorkMarginIndexes,true,false);
			break;
		case COMMAND_REDUCE:
			TestAndReduce(&WorkMarginIndexes,&ptd);
			break;
	}
}
bool MarginPerceptron::OnInit()
{
	UInt32					tr,count;
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
	// adaug si indexii
	for (tr=0,count,list=RecordIndexes.GetList();tr<RecordIndexes.Len();tr++,list++)
	{
		if (con->GetRecordLabel(label,*list)==false)
		{
			notif->Error("[%s] -> Unable to read record #d label",ObjectName,(*list));
			return false;
		}
		if ((label==1.0) && (MarginType==MARGIN_POZITIVE))
			MarginIndexes.Push(*list);
		if ((label==-1.0) && (MarginType==MARGIN_NEGATIVE))
			MarginIndexes.Push(*list);
	}
	notif->Info("[%s] -> Margin indexes created",ObjectName);
	// aloc si date pentru extraIntervale
	for (tr=0;tr<threadsCount;tr++)
	{
		if ((ptData[tr].ExtraData = new GML::Utils::Interval[2])==NULL)
		{
			notif->Error("[%s] -> Unable to alloc interval for thread %d",ObjectName,tr);
			return false;			
		}
		((GML::Utils::Interval *)ptData[tr].ExtraData)[0]=ptData[tr].Range;
	}
	// splituim si pentru work
	tempRange.Set(0,MarginIndexes.Len());
	if (SplitInterval(ptData,threadsCount,tempRange)==false)
		return false;
	for (tr=0;tr<threadsCount;tr++)
	{
		((GML::Utils::Interval *)ptData[tr].ExtraData)[1]=ptData[tr].Range;
	}
	return true;
}
bool MarginPerceptron::TestAndReduce(GML::Utils::Indexes *indexes,PerceptronThreadData *ptd)
{
	UInt32	*ptrIndex = indexes->GetList();
	UInt32	count = ptd->Range.Size(),tr;
	UInt32	nrFeatures = con->GetFeatureCount();
	UInt32	diff = 0;
	double	*w = ptd->Primary.Weight;
	double	*b = ptd->Primary.Bias;
	
	if (!useB)
		(*b)=0;
	
	ptrIndex+=ptd->Range.Start;

	for (tr=0;tr<count;tr++)
	{
		if (con->GetRecord(ptd->Record,ptrIndex[tr])==false)
		{
			notif->Error("[%s] -> (TEST)::Error reading record #%d from thread #%d",ObjectName,(ptrIndex[tr]),ptd->ID);
			return false;
		}
		if (GML::ML::VectorOp::IsPerceptronTrained(ptd->Record.Features,w,nrFeatures,*b,ptd->Record.Label)==true)
		{
			diff++;
		} else {
			ptrIndex[tr-diff]=ptrIndex[tr];
		}
	}
	ptd->Range.End-=diff;

	return true;	
}

bool MarginPerceptron::PerformTrainIteration()
{
	UInt32	tr,errorCount;
	
	// paralel mode
	for (tr=0;tr<threadsCount;tr++)
		ptData[tr].Range = ((GML::Utils::Interval *)ptData[tr].ExtraData)[0];
	ExecuteParalelCommand(COMMAND_TRAIN);
	
	// aditie de date	
	for (tr=0;tr<threadsCount;tr++)
		FullData.Primary.Add(ptData[tr].Delta);	
	
	// refac indexii
	if (MarginIndexes.CopyTo(WorkMarginIndexes)==false)
	{
		notif->Error("[%s] -> Unable to copy indexes ...",ObjectName);
		return false;
	}
	// reducere efectiva
	for (tr=0;tr<threadsCount;tr++)
		ptData[tr].Range = ((GML::Utils::Interval *)ptData[tr].ExtraData)[1];
	do
	{
		ExecuteParalelCommand(COMMAND_TRAIN_ON_MARGIN);
		for (tr=0;tr<threadsCount;tr++)
			FullData.Primary.Add(ptData[tr].Delta);	
		ExecuteParalelCommand(COMMAND_REDUCE);
		for (tr=0,errorCount=0;tr<threadsCount;tr++)
			errorCount+=ptData[tr].Range.Size();
	} while (errorCount>MaxError);

	return true;
}
bool MarginPerceptron::PerformTestIteration()
{
	UInt32	tr;

	for (tr=0;tr<threadsCount;tr++)
		ptData[tr].Range = ((GML::Utils::Interval *)ptData[tr].ExtraData)[0];
	ExecuteParalelCommand(COMMAND_TEST);
	FullData.Res.Clear();
	for (tr=0;tr<threadsCount;tr++)
		FullData.Res.Add(&ptData[tr].Res);
	FullData.Res.Compute();

	return true;
}
