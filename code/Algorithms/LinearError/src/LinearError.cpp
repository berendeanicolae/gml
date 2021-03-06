#include "LinearError.h"

LinearError::LinearError()
{
	ObjectName = "LinearError";

	//Add extra commands here
	SetPropertyMetaData("Command","!!LIST:None=0,Compute!!");
	LinkPropertyToDouble("LearningRate",LearningRate,0.1,"Learning rate used ... ");
	LinkPropertyToBool  ("UseBias",UseBias,false,"");
	LinkPropertyToUInt32("MaxIterations",MaxIterations,100,"");
	LinkPropertyToUInt32("SaveData",SaveData,SAVE_DATA_WhenAlgorithmEnds,"!!LIST:WhenAlgorithmEnds=0,AfterEachIteration!!");
	LinkPropertyToUInt32("FilterBy",Filter,FILTER_NONE,"!!LIST:None=0,Value,Percentage,UniformPercentage!!");
	LinkPropertyToDouble("FilterMin",Min,0);
	LinkPropertyToDouble("FilterMax",Max,1);
	LinkPropertyToString("ResultFileName",ResultFileName,"");
}
bool LinearError::Init()
{
	if (InitConnections()==false)
		return false;
	if (InitThreads()==false)
		return false;
	if (SplitMLThreadDataRange(con->GetRecordCount())==false)
		return false;
	if (con->CreateMlRecord(MainRecord)==false)
	{
		notif->Error("[%s] -> Unable to create MainRecord",ObjectName);
		return false;
	}
	if ((Weight = new double[con->GetFeatureCount()])==NULL)
	{
		notif->Error("[%s] -> Unable to create Weight vector",ObjectName);
		return false;
	}
	if ((RecordErrors = new UInt32[con->GetRecordCount()])==NULL)
	{
		notif->Error("[%s] -> Unable to create Errors vector",ObjectName);
		return false;
	}
	MEMSET(Weight,0,sizeof(double)*	con->GetFeatureCount());
	MEMSET(RecordErrors,0,sizeof(UInt32) * con->GetRecordCount());
	Bias = 0;
	return true;
}
void LinearError::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand)
{
	switch (threadCommand)
	{
		case THREAD_COMMAND_NONE:
			// do nothing
			return;
		case THREAD_COMMAND_TRAIN:
			TrainIteration(thData);
			break;		
	};
}
bool LinearError::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	LinearErrorThreadData	*obj_td = new LinearErrorThreadData();
	if (obj_td==NULL)
		return false;
	if ((obj_td->delta = new double[con->GetFeatureCount()])==NULL)
		return false;

	thData.Context = obj_td;
	return true;
}
bool LinearError::TrainIteration(GML::Algorithm::MLThreadData &thData)
{
	double 	*delta = ((LinearErrorThreadData *)thData.Context)->delta;
	double	*deltaBias = &((LinearErrorThreadData *)thData.Context)->deltaBias;
	UInt32	*countErrors = &((LinearErrorThreadData *)thData.Context)->countErrors;
	UInt32	*countPozitive = &((LinearErrorThreadData *)thData.Context)->countPozitive;
	UInt32	*countNegative = &((LinearErrorThreadData *)thData.Context)->countNegative;
	UInt32	nrRecords = con->GetRecordCount();
	UInt32	nrFeatures = con->GetFeatureCount();
	double	sum,error;
	
	MEMSET(delta,0,sizeof(double)*nrFeatures);
	(*deltaBias) = 0.0;
	(*countErrors) = 0;
	(*countPozitive) = 0;
	(*countNegative) = 0;
	
	for (UInt32 tr=thData.ThreadID;(tr<nrRecords) && (StopAlgorithm==false);tr+=threadsCount)
	{
		if (con->GetRecord(thData.Record,tr)==false)
		{
			notif->Error("[%s] -> Unable to read record %d",ObjectName,tr);
			return false;
		}
		sum = GML::ML::VectorOp::ComputeVectorsSum(thData.Record.Features,Weight,nrFeatures);
		if (UseBias)
			sum+=Bias;
		if ((sum * thData.Record.Label)<=0)
		{
			error = thData.Record.Label * LearningRate;
			GML::ML::VectorOp::AdjustPerceptronWeights(thData.Record.Features,delta,nrFeatures,error);
			if (UseBias)
				(*deltaBias) += error;			
			RecordErrors[tr]++;
			if (thData.Record.Label==1)
				(*countPozitive)++;
			else
				(*countNegative)++;
			(*countErrors)++;
		}
	}
	return true;
}
bool LinearError::SaveResultTable(char *fileName)
{
	GML::Utils::File		f;
	GML::Utils::GString		fName,temp,hash;
	UInt32					tr,count = 0,countPozitive = 0,countNegative=0;
	GML::DB::RecordHash		rHash;
	double					Label;
	double					v_min,v_max,max_value;
	double					max_value_poz,max_value_neg;
	double					v_min_poz,v_min_neg,v_max_poz,v_max_neg;
	
	if (temp.Create(0x10000)==false)
	{
		notif->Error("[%s] -> Unable to alloc memory for cache",ObjectName);
		return false;
	}
	if (f.Create(fileName)==false)
	{
		notif->Error("[%s] -> Unable to create: %s",ObjectName,fileName);
		return false;
	}
	switch (Filter)
	{
		case FILTER_UNIFORM_PERCENTAGE:
			max_value_poz = max_value_neg = 0;
			for (tr=0;tr<con->GetRecordCount();tr++)
			{
				if (con->GetRecordLabel(Label,tr)==false)
				{
					notif->Error("[%s] -> Unable to read label for #%d",ObjectName,tr);
					return false;
				}			
				if ((Label==1) && (RecordErrors[tr]>max_value_poz))
					max_value_poz = RecordErrors[tr];	
				if ((Label!=1) && (RecordErrors[tr]>max_value_neg))
					max_value_neg = RecordErrors[tr];					
			}
			notif->Info("[%s] -> Max number of errors so far : Poz:%d , Neg:%d",ObjectName,(UInt32)max_value_poz,(UInt32)max_value_neg);
			v_min_poz = Min * max_value_poz;
			v_min_neg = Min * max_value_neg;
			v_max_poz = Max * max_value_poz;
			v_max_neg = Max * max_value_neg;			
			break;
		case FILTER_PERCENTAGE:
			max_value = 0;
			for (tr=0;tr<con->GetRecordCount();tr++)
				if (RecordErrors[tr]>max_value)
					max_value = RecordErrors[tr];
			notif->Info("[%s] -> Max number of errors so far : %d",ObjectName,(UInt32)max_value);
			v_min = Min * max_value;
			v_max = Max * max_value;		
			break;
		default:
			v_min = Min;
			v_max = Max;
			break;
	};
	
	for (tr=0;tr<con->GetRecordCount();tr++)
	{
		if ((Filter!=FILTER_NONE) && (Filter!=FILTER_UNIFORM_PERCENTAGE))
		{
			if ((double)RecordErrors[tr]<v_min)
				continue;
			if ((double)RecordErrors[tr]>v_max)
				continue;				
		}
		if (con->GetRecordLabel(Label,tr)==false)
		{
			notif->Error("[%s] -> Unable to read label for #%d",ObjectName,tr);
			return false;
		}
		if (Filter==FILTER_UNIFORM_PERCENTAGE)
		{
			if (Label==1)
			{
				if ((double)RecordErrors[tr]<v_min_poz)
					continue;
				if ((double)RecordErrors[tr]>v_max_poz)
					continue;				
			} else {
				if ((double)RecordErrors[tr]<v_min_neg)
					continue;
				if ((double)RecordErrors[tr]>v_max_neg)
					continue;			
			}
		}
		if (con->GetRecordHash(rHash,tr)==false)
		{
			notif->Error("[%s] -> Unable to read record hash for #%d",ObjectName,tr);
			return false;
		}

		if (rHash.ToString(hash)==false)
		{
			notif->Error("[%s] -> Unable to convert record hash for #%d",ObjectName,tr);
			return false;
		}
		if (temp.AddFormated("%s|%.3lf|%d\n",hash.GetText(),Label,RecordErrors[tr])==false)
		{
			notif->Error("[%s] -> Unable to create log entry for record #%d",ObjectName,tr);
			return false;
		}
		count++;
		if (Label==1)
			countPozitive++;
		else
			countNegative++;
		if (temp.Len()>64000)
		{
			if (f.Write(temp,temp.Len())==false)
			{
				notif->Error("[%s] -> Unable to write to %s",ObjectName,fileName);
				return false;
			}
			temp.Truncate(0);
			temp.Set("");
		}
	}
	if (f.Write(temp,temp.Len())==false)
	{
		notif->Error("[%s] -> Unable to write to %s",ObjectName,fileName);
		return false;
	}
	f.Close();
	notif->Info("[%s] -> %d records written in %s (Poz:%d , Neg:%d)",ObjectName,count,fileName,countPozitive,countNegative);
	return true;
}
void LinearError::ComputeErrors()
{
	UInt32 					countErrors=0;
	UInt32 					countPozitive=0;
	UInt32 					countNegative=0;
	GML::Utils::GString		temp;
	
	for (UInt32 tr=0;tr<MaxIterations;tr++)
	{
		ExecuteParalelCommand(THREAD_COMMAND_TRAIN);
		// aditie
		countErrors = countPozitive = countNegative = 0;
		for (UInt32 gr=0;gr<threadsCount;gr++)
		{
			GML::ML::VectorOp::AddVectors(Weight,((LinearErrorThreadData *)ThData[gr].Context)->delta,con->GetFeatureCount());
			if (UseBias)
				Bias+=((LinearErrorThreadData *)ThData[gr].Context)->deltaBias;
			countErrors += ((LinearErrorThreadData *)ThData[gr].Context)->countErrors;
			countPozitive += ((LinearErrorThreadData *)ThData[gr].Context)->countPozitive;
			countNegative += ((LinearErrorThreadData *)ThData[gr].Context)->countNegative;
		}
		notif->Info("[%s] -> It:%5d  Errors:%8d  Poz:%8d  Neg:%8d",ObjectName,tr+1,countErrors,countPozitive,countNegative);
		if (SaveData == SAVE_DATA_AfterEachIteration)
		{
			temp.SetFormated("%s_IT_%05d___ERR_%07d.log",ResultFileName.GetText(),tr+1,countErrors);
			SaveResultTable(temp.GetText());
		}
		if (countErrors==0)
			break;
	}
	if (SaveData == SAVE_DATA_WhenAlgorithmEnds)
	{
		temp.SetFormated("%s_FINAL___ERR_%07d.log",ResultFileName.GetText(),countErrors);
		SaveResultTable(temp.GetText());
	}
	
}
void LinearError::OnExecute()
{
	switch (Command)
	{
		case COMMAND_NONE:
			notif->Error("[%s] -> Nothing to do , select another command ",ObjectName);
			break;
		case COMMAND_COMPUTE:
			ComputeErrors();
			break;
		default:
			notif->Error("[%s] -> Unknown command ID: %d",ObjectName,Command);
			break;
	}	
}
