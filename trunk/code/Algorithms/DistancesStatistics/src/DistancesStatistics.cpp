#include "DistancesStatistics.h"

DistancesStatistics::DistancesStatistics()
{
	ObjectName = "DistancesStatistics";

	//Add extra commands here
	SetPropertyMetaData("Command","!!LIST:None=0,Compute!!");
	
	LinkPropertyToDouble("HistogramMinValue",HistogramMinValue,0.0,"Minim value for histogram listing");
	LinkPropertyToDouble("HistogramMaxValue",HistogramMaxValue,100.0,"Maxim value for histogram listing");
	LinkPropertyToDouble("HistogramStep",HistogramStep,1.0,"Step in Histogram (must be bigger than 0.0)");
	LinkPropertyToString("ResultFile",ResultFile,"","Name of the where the results will be written");
	LinkPropertyToBool  ("UseWeightsForFeatures",UseWeightsForFeatures,false,"Specifyes if weights for features should be used.");
	LinkPropertyToUInt32("Method",Method,0,"!!LIST:PositiveToNegative=0,PositiveToPositive,NegativeToPositive,NegativeToNegative,MinPositiveToNegative,MinPositiveToPositive,MinNegativeToPositive,MinNegativeToNegative,FeaturesCount!!");
	LinkPropertyToString("FeaturesWeightFile",FeaturesWeightFile,"","Name of the file that contains the weights for features!");
	LinkPropertyToBool  ("Ignore0ValuesInHistogram",Ignore0ValuesInHistogram,true,"If set , 0 values in histogram will not be saved in the result file");
	AddDistanceProperties();
}
bool DistancesStatistics::Init()
{
	if (InitConnections()==false)
		return false;
	if (InitThreads()==false)
		return false;
	if (con->CreateMlRecord(MainRecord)==false)
	{
		notif->Error("[%s] -> Unable to create MainRecord",ObjectName);
		return false;
	}
	if (UseWeightsForFeatures)
	{
		if (LoadFeatureWeightFile()==false)
			return false;
	}
	if (CreateHistogram(Histogram,true)==false)
		return false;
	if (ResultFile.Len()==0)
	{
		notif->Error("[%s] -> Missing 'ResultFile' parameter (or not set)",ObjectName);
		return false;	
	}
	switch (Method)
	{
		case METHOD_PositiveToNegative:
		case METHOD_PositiveToPositive:
		case METHOD_NegativeToPositive:
		case METHOD_NegativeToNegative:
		case METHOD_MinPositiveToNegative:
		case METHOD_MinPositiveToPositive:
		case METHOD_MinNegativeToPositive:
		case METHOD_MinNegativeToNegative:		
			if (CreatePozitiveAndNegativeIndexes()==false)
				return false;
			break;
	}
	// Add extra initialization here
	return true;
}
double DistancesStatistics::GetDistance(GML::ML::MLRecord &r1,GML::ML::MLRecord &r2)
{
	if (UseWeightsForFeatures)
		return GML::Algorithm::IMLAlgorithm::GetDistance(r1,r2,featWeight);
	else
		return GML::Algorithm::IMLAlgorithm::GetDistance(r1,r2);
}
bool DistancesStatistics::CreatePozitiveAndNegativeIndexes()
{
	UInt32	tr;
	UInt32	pozitiveCount,negativeCount,nrRecords;
	double	label;
	bool	res;

	notif->Info("[%s] -> Creating Pozitive and Negative indexes ... ",ObjectName);
	pozitiveCount = negativeCount = 0;
	nrRecords = con->GetRecordCount();
	
	for (tr=0;tr<nrRecords;tr++)
	{
		if (con->GetRecordLabel(label,tr)==false)
		{
			notif->Error("[%s] -> Unable to read label for record #%d",ObjectName,tr);
			return false;
		}
		if (label==1.0)
			pozitiveCount++;
		else
			negativeCount++;
	}
	notif->Info("[%s] -> Pozitive = %d ,Negative = %d",ObjectName,pozitiveCount,negativeCount);
	if (indexesPozitive.Create(pozitiveCount)==false)
	{
		notif->Error("[%s] -> Unable to alloc %d pozitive indexes",ObjectName,pozitiveCount);
		return false;
	}
	if (indexesNegative.Create(negativeCount)==false)
	{
		notif->Error("[%s] -> Unable to alloc %d negative indexes",ObjectName,negativeCount);
		return false;
	}
	// adaugam si indexii
	for (tr=0;tr<nrRecords;tr++)
	{
		if (con->GetRecordLabel(label,tr)==false)
		{
			notif->Error("[%s] -> Unable to read label for record #%d",ObjectName,tr);
			return false;
		}
		if (label==1.0)
			res = indexesPozitive.Push(tr);
		else
			res = indexesNegative.Push(tr);
		if (res==false)
		{
			notif->Error("[%s] -> Unable to add index #%d",ObjectName,tr);
			return false;
		}		
	}
	notif->Info("[%s] -> Pozitive and Negative indexes created !",ObjectName);
	return true;
}
bool DistancesStatistics::ComputeHistogram(GML::Algorithm::MLThreadData &thData,GML::Utils::Indexes &class1,GML::Utils::Indexes &class2)
{
	UInt32								tr,gr,featuresCount,class1Count,class2Count,idxPoz,idxNeg,histoIndex;
	DistancesStatisticsThreadData*		dt = (DistancesStatisticsThreadData *)thData.Context;
	double								dist;	
	

	featuresCount = con->GetFeatureCount();
	class1Count = class1.Len();
	class2Count = class2.Len();
	
	for (tr=0;tr<dt->Histogram.Len();tr++)
		dt->Histogram[tr]=0;
		
	if (thData.ThreadID==0)
		notif->StartProcent("[%s] -> Computing ... ",ObjectName);
	for (tr=thData.ThreadID;tr<class1Count;tr+=threadsCount)
	{
		idxPoz = class1.Get(tr);
		if (con->GetRecord(thData.Record,idxPoz)==false)
		{
			notif->Error("[%s] -> Unable to read record #%d",ObjectName,idxPoz);
			return false;
		}
		for (gr=0;gr<class2Count;gr++)
		{
			idxNeg = class2.Get(gr);
			if (con->GetRecord(dt->SecRec,idxNeg)==false)
			{
				notif->Error("[%s] -> Unable to read record #%d",ObjectName,idxNeg);
				return false;
			}
			dist = GetDistance(thData.Record,dt->SecRec);
			histoIndex = ValueToHistogramIndex(dist);
			dt->Histogram[histoIndex]++;
		}
		if (thData.ThreadID==0)
			notif->SetProcent(tr,class1Count);
	}
	if (thData.ThreadID==0)
		notif->EndProcent();
	return true;
}

bool DistancesStatistics::ComputeFeatureCountHistogram(GML::Algorithm::MLThreadData &thData)
{
	UInt32								tr,gr,featuresCount,histoIndex,recCount,count,id;
	DistancesStatisticsThreadData*		dt = (DistancesStatisticsThreadData *)thData.Context;
	double*								ptrFeat;
	
	

	featuresCount = con->GetFeatureCount();
	recCount = con->GetRecordCount();
	
	for (tr=0;tr<dt->Histogram.Len();tr++)
		dt->Histogram[tr]=0;
		
	if (thData.ThreadID==0)
		notif->StartProcent("[%s] -> Computing ... ",ObjectName);
	id = 0;
	for (tr=thData.ThreadID;tr<recCount;tr+=threadsCount,id++)
	{		
		if (con->GetRecord(thData.Record,tr)==false)
		{
			notif->Error("[%s] -> Unable to read record #%d",ObjectName,tr);
			return false;
		}
		count = 0;
		ptrFeat = thData.Record.Features;
		gr = featuresCount;
		while (gr>0)		
		{
			if ((*ptrFeat)!=0.0)
				count++;
			ptrFeat++;
			gr--;
		}
		histoIndex = ValueToHistogramIndex(count);
		dt->Histogram[histoIndex]++;		
		
		if ((thData.ThreadID==0) && ((id % 10000)==0))
			notif->SetProcent(tr,recCount);
	}
	if (thData.ThreadID==0)
		notif->EndProcent();
	return true;
}

bool DistancesStatistics::ComputeHistogramMin(GML::Algorithm::MLThreadData &thData,GML::Utils::Indexes &class1,GML::Utils::Indexes &class2)
{
	UInt32								tr,gr,featuresCount,class1Count,class2Count,idxPoz,idxNeg,histoIndex;
	DistancesStatisticsThreadData*		dt = (DistancesStatisticsThreadData *)thData.Context;
	double								dist,minDist;	
	

	featuresCount = con->GetFeatureCount();
	class1Count = class1.Len();
	class2Count = class2.Len();
	
	for (tr=0;tr<dt->Histogram.Len();tr++)
		dt->Histogram[tr]=0;
		
	if (thData.ThreadID==0)
		notif->StartProcent("[%s] -> Computing ... ",ObjectName);
	for (tr=thData.ThreadID;tr<class1Count;tr+=threadsCount)
	{
		idxPoz = class1.Get(tr);
		if (con->GetRecord(thData.Record,idxPoz)==false)
		{
			notif->Error("[%s] -> Unable to read record #%d",ObjectName,idxPoz);
			return false;
		}
		for (gr=0;gr<class2Count;gr++)
		{
			idxNeg = class2.Get(gr);
			if (con->GetRecord(dt->SecRec,idxNeg)==false)
			{
				notif->Error("[%s] -> Unable to read record #%d",ObjectName,idxNeg);
				return false;
			}
			dist = GetDistance(thData.Record,dt->SecRec);
			if (gr==0) {
				minDist = dist;
			} else {
				if (dist<minDist)
					minDist = dist;
			}
		}
		histoIndex = ValueToHistogramIndex(minDist);
		dt->Histogram[histoIndex]++;		
		if (thData.ThreadID==0)
			notif->SetProcent(tr,class1Count);
	}
	if (thData.ThreadID==0)
		notif->EndProcent();
	return true;
}

bool DistancesStatistics::LoadFeatureWeightFile()
{
	GML::Utils::AttributeList	attrList;
	UInt32						tr;

	if ((featWeight = new double[con->GetFeatureCount()])==NULL)
	{
		notif->Error("[%s] -> Unable to alloc featWeight[%d]",ObjectName,con->GetFeatureCount());
		return false;
	}
	if (attrList.Load(FeaturesWeightFile.GetText())==false)
	{
		notif->Error("[%s] -> Unable to load FeaturesWeightFile : %s",ObjectName,FeaturesWeightFile.GetText());
		return false;
	}
	if (attrList.Update("Weight",featWeight,sizeof(double)*con->GetFeatureCount())==false)
	{
		notif->Error("[%s] -> Unable to update 'Weight' property from %s",ObjectName,FeaturesWeightFile.GetText());
		return false;
	}

	notif->Info("[%s] -> %s loaded ok ",ObjectName,FeaturesWeightFile.GetText());
	// facem si un mic test

	for (tr=0;tr<con->GetFeatureCount();tr++)
	{
		if (featWeight[tr]<=0.0)
		{
			notif->Error("[%s] -> Feature Weight %d is lower than 0.0 (%.4lf)",ObjectName,tr,featWeight[tr]);
		}	
	}
	return true;
}
bool DistancesStatistics::CreateHistogram(GML::Utils::GTFVector<UInt32> &histoObject,bool showFinalMessage)
{
	if (HistogramStep<=0)
	{
		notif->Error("[%s] -> Invalid 'HistogramStep' property (should be bigger than 0.0)",ObjectName);
		return false;	
	}
	if (HistogramMinValue>HistogramMaxValue)
	{
		notif->Error("[%s] -> Invalid 'HistogramMinValue' property (should be smaller than 'HistogramMaxValue' property)",ObjectName);
		return false;		
	}
	// calculez cate elemente sunt
	double interval = HistogramMaxValue-HistogramMinValue;
	UInt32 elements = (UInt32)((interval/HistogramStep)+3); // +3 astfel (+1 pt. eroare, +1 >Max, +1 <Min)
	if (histoObject.Create(elements,true)==false)
	{
		notif->Error("[%s] -> Unable to alloc %d elements for Histogram object",ObjectName,elements);
		return false;	
	}
	for (UInt32 tr=0;tr<elements;tr++)
		histoObject[tr] = 0;
	if (showFinalMessage)
		notif->Info("[%s] -> Histogram created (Min:%lf,Max:%lf,Step:%lf,Elements:%d)",ObjectName,HistogramMinValue,HistogramMaxValue,HistogramStep,elements);
	return true;
}
UInt32 DistancesStatistics::ValueToHistogramIndex(double value)
{
	if (value<HistogramMinValue)
		return 0;
	if (value>HistogramMaxValue)
		return Histogram.Len()-1;
	return ((UInt32)((value-HistogramMinValue)/HistogramStep))+1;
}
void DistancesStatistics::MergeThreadHistograms()
{
	UInt32	tr,gr;
	
	for (tr=0;tr<Histogram.Len();tr++)
	{
		Histogram[tr]=0;
		for (gr=0;gr<threadsCount;gr++)
		{
			Histogram[tr] += ((DistancesStatisticsThreadData *)(ThData[gr].Context))->Histogram	[tr];
		}
	}
}
void DistancesStatistics::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand)
{
	switch (threadCommand)
	{
		case THREAD_COMMAND_NONE:
			// do nothing
			return;
		case THREAD_COMMAND_COMPUTE_PositiveToNegative:
			ComputeHistogram(thData,indexesPozitive,indexesNegative);
			break;
		case THREAD_COMMAND_COMPUTE_PositiveToPositive:
			ComputeHistogram(thData,indexesPozitive,indexesPozitive);
			break;
		case THREAD_COMMAND_COMPUTE_NegativeToPositive:
			ComputeHistogram(thData,indexesNegative,indexesPozitive);
			break;
		case THREAD_COMMAND_COMPUTE_NegativeToNegative:
			ComputeHistogram(thData,indexesNegative,indexesNegative);
			break;
			
		case THREAD_COMMAND_COMPUTE_MinPositiveToNegative:
			ComputeHistogramMin(thData,indexesPozitive,indexesNegative);
			break;
		case THREAD_COMMAND_COMPUTE_MinPositiveToPositive:
			ComputeHistogramMin(thData,indexesPozitive,indexesPozitive);
			break;
		case THREAD_COMMAND_COMPUTE_MinNegativeToPositive:
			ComputeHistogramMin(thData,indexesNegative,indexesPozitive);
			break;
		case THREAD_COMMAND_COMPUTE_MinNegativeToNegative:
			ComputeHistogramMin(thData,indexesNegative,indexesNegative);
			break;			
			
		case THREAD_COMMAND_COMPUTE_FeaturesCount:
			ComputeFeatureCountHistogram(thData);
			break;
		// add extra thread command processes here
	};
}
bool DistancesStatistics::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	DistancesStatisticsThreadData	*obj_td = new DistancesStatisticsThreadData();
	if (obj_td==NULL)
		return false;
	if (CreateHistogram(obj_td->Histogram,false)==false)
		return false;
	if (con->CreateMlRecord(obj_td->SecRec)==false)
		return false;
	thData.Context = obj_td;
	return true;
}
bool DistancesStatistics::SaveHistogram()
{
	GML::Utils::File	f;
	GML::Utils::GString	tmp;
	UInt32				tr;
	double				sum = 0.0,i1,i2,proc,cum = 0.0,pCum;
	bool				isDouble;
	
	if (f.Create(ResultFile.GetText())==false)
	{
		notif->Error("[%s] -> Unable to create result file: %s",ObjectName,ResultFile.GetText());
		return false;
	}
	tmp.Create(64000);
	tmp.Set("Interval           |   Count  |  Procent |Cumulative\n");
	tmp.Add("====================================================\n");
	
	for (tr=0;tr<Histogram.Len();tr++)
		sum+=Histogram[tr];
	notif->Info("[%s] -> Total number of distances computed: (%d)",ObjectName,(int)sum);
	
	isDouble = true;
	if ((HistogramStep == (double)((int)(HistogramStep))) && (HistogramMinValue == (double)((int)(HistogramMinValue))))
		isDouble = false;
		
	proc = 100.0 * (((double)Histogram[0])/sum);		
	if (isDouble)
			tmp.AddFormatedEx("Less than %{double,Z2,L9}|%{uint32,R10}|%{double,Z3,R9}%|%{double,Z3,R9}%\n",HistogramMinValue,Histogram[0],proc,proc);
		else
			tmp.AddFormatedEx("Less than %{uint32,L9}|%{uint32,R10}|%{double,Z3,R9}%|%{double,Z3,R9}%\n",(UInt32)HistogramMinValue,Histogram[0],proc,proc);
	cum = Histogram[0];		
	for (UInt32 tr=1;(tr<Histogram.Len());tr++)
	{		
		i1 = (double)(HistogramMinValue+(double)(tr-1)*HistogramStep);
		if (i1>HistogramMaxValue)
			break;
		i2 = i1+HistogramStep;
		proc = 100.0 * (((double)Histogram[tr])/sum);
		cum += Histogram[tr];
		pCum = 100.0 * (cum / sum);
		if ((Ignore0ValuesInHistogram) && (Histogram[tr]==0))
			continue;
		if (isDouble)
			tmp.AddFormatedEx("%{double,Z2,R8} - %{double,Z2,R8}|%{uint32,R10}|%{double,Z3,R9}%|%{double,Z3,R9}%\n",i1,i2,Histogram[tr],proc,pCum);
		else
			tmp.AddFormatedEx("%{uint32,R8} - %{uint32,R8}|%{uint32,R10}|%{double,Z3,R9}%|%{double,Z3,R9}%\n",(UInt32)i1,(UInt32)i2,Histogram[tr],proc,pCum);
		if (tmp.Len()>60000)
		{
			if (f.Write(tmp.GetText(),tmp.Len())==false)
			{
				notif->Error("[%s] -> Unable to write to result file: %s",ObjectName,ResultFile.GetText());
				f.Close();
				DeleteFile(ResultFile.GetText());
				return false;		
			}
			tmp.Set("");
		}		
	}
	proc = 100.0 * (((double)Histogram[Histogram.Len()-1])/sum);	
	pCum = 100.0;
	if (isDouble)
			tmp.AddFormatedEx("Bigger than %{double,Z2,L7}|%{uint32,R10}|%{double,Z3,R9}%|%{double,Z3,R9}%\n",HistogramMaxValue,Histogram[Histogram.Len()-1],proc,pCum);
		else
			tmp.AddFormatedEx("Bigger than %{uint32,L7}|%{uint32,R10}|%{double,Z3,R9}%|%{double,Z3,R9}%\n",(UInt32)HistogramMaxValue,Histogram[Histogram.Len()-1],proc,pCum);	
	if (f.Write(tmp.GetText(),tmp.Len())==false)
	{
		notif->Error("[%s] -> Unable to write to result file: %s",ObjectName,ResultFile.GetText());
		f.Close();
		DeleteFile(ResultFile.GetText());
		return false;		
	}	
	notif->Info("[%s] -> Result file (%s) created !",ObjectName,ResultFile.GetText());
	f.Close();
	return true;
}
void DistancesStatistics::Compute()
{
	switch (Method)
	{
		case METHOD_PositiveToNegative:
			ExecuteParalelCommand(THREAD_COMMAND_COMPUTE_PositiveToNegative);
			MergeThreadHistograms();
			SaveHistogram();
			break;
		case METHOD_PositiveToPositive:
			ExecuteParalelCommand(THREAD_COMMAND_COMPUTE_PositiveToPositive);
			MergeThreadHistograms();
			SaveHistogram();
			break;
		case METHOD_NegativeToPositive:
			ExecuteParalelCommand(THREAD_COMMAND_COMPUTE_NegativeToPositive);
			MergeThreadHistograms();
			SaveHistogram();
			break;
		case METHOD_NegativeToNegative:
			ExecuteParalelCommand(THREAD_COMMAND_COMPUTE_NegativeToNegative);
			MergeThreadHistograms();
			SaveHistogram();
			break;	

		case METHOD_MinPositiveToNegative:
			ExecuteParalelCommand(THREAD_COMMAND_COMPUTE_MinPositiveToNegative);
			MergeThreadHistograms();
			SaveHistogram();
			break;
		case METHOD_MinPositiveToPositive:
			ExecuteParalelCommand(THREAD_COMMAND_COMPUTE_MinPositiveToPositive);
			MergeThreadHistograms();
			SaveHistogram();
			break;
		case METHOD_MinNegativeToPositive:
			ExecuteParalelCommand(THREAD_COMMAND_COMPUTE_MinNegativeToPositive);
			MergeThreadHistograms();
			SaveHistogram();
			break;
		case METHOD_MinNegativeToNegative:
			ExecuteParalelCommand(THREAD_COMMAND_COMPUTE_MinNegativeToNegative);
			MergeThreadHistograms();
			SaveHistogram();
			break;	
		
		case METHOD_ComputeFeaturesCount:
			ExecuteParalelCommand(THREAD_COMMAND_COMPUTE_FeaturesCount);
			MergeThreadHistograms();
			SaveHistogram();
			break;				
	}
}
void DistancesStatistics::OnExecute()
{
	switch (Command)
	{
		case COMMAND_NONE:
			notif->Error("[%s] -> Nothing to do , select another command ",ObjectName);
			break;
		case COMMAND_COMPUTE:
			Compute();
			break;
		default:
			notif->Error("[%s] -> Unknown command ID: %d",ObjectName,Command);
			break;
	}	
}
