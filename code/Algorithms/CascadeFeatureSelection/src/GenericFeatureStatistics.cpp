#include "GenericFeatureStatistics.h"

int cmpFunction(FeatureScore &scor1,FeatureScore &scor2)
{
	if (scor1.Score>scor2.Score)
		return 1;
	if (scor1.Score<scor2.Score)
		return -1;
	return 0;	
}
//==============================================================================
void FeatureCounters::Reset()
{
	CountPozitive = 0;
	CountNegative = 0;
	CountTotalPozitive = 0;
	CountTotalNegative = 0;
}
void FeatureCounters::Add(FeatureCounters &fc)
{
	CountPozitive += fc.CountPozitive;
	CountNegative += fc.CountNegative;
	CountTotalPozitive += fc.CountTotalPozitive;
	CountTotalNegative += fc.CountTotalNegative;
}
//===================================
void FeatureInformationFromFeatureCounters(FeatureCounters &fc, GML::ML::FeatureInformation &fi)
{
	fi.totalPozitive = fc.CountTotalPozitive;
	fi.totalNegative = fc.CountTotalNegative;
	fi.countNegative = fc.CountNegative;
	fi.countPozitive = fc.CountPozitive;
}
//===================================
Stats::Stats()
{
	fnCompute = NULL;
}
Stats::Stats(Stats &ref)
{
	fnCompute = ref.fnCompute;
	this->Name.Set(ref.Name.GetText());
}
GenericFeatureStatistics::GenericFeatureStatistics()
{
	UInt32					tr;
	UInt32					funcCount;

	ObjectName = "GenericFeatureStatistics";

	//Add extra commands here
	SetPropertyMetaData("Command","!!LIST:None=0,Compute!!");	
	LinkPropertyToString("ResultFile",ResultFileName,"","Name of the output file");
	
	//Add MeasureFunctions from FeatStats from GmlLib
	funcCount = GML::ML::FeatStatsFunctions::GetFunctionsCount();
	for (tr=0;tr<funcCount;tr++)
		AddNewStatFunction(GML::ML::FeatStatsFunctions::GetFunctionName(tr),GML::ML::FeatStatsFunctions::GetFunctionPointer(tr));
		
	computeMethod.Set("!!LIST:None=0xFFFF");
	for (tr=0;tr<StatsData.Len();tr++)
	{
		char *text = StatsData[tr].Name.GetText();
		computeMethod.AddFormated(",%s=%d",text,tr);
	}
	computeMethod.Add("!!");
	LinkPropertyToUInt32("FeatStatMethod",computeMethodIndex,0xFFFF,computeMethod.GetText());
	
	callThreadComputeExtraDataFunction = false;
}
bool GenericFeatureStatistics::Init()
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
	if (FeatCounters.Create(con->GetFeatureCount(),true)==false)
	{
		notif->Error("[%s] -> Unable to create FeatCounters main list !",ObjectName);
		return false;	
	}
	if (FeatScores.Create(con->GetFeatureCount(),true)==false)
	{
		notif->Error("[%s] -> Unable to create FeatScores main list !",ObjectName);
		return false;		
	}
	if ((RemovedRecords = new bool[con->GetRecordCount()])==NULL)
	{
		notif->Error("[%s] -> Unable to create RemovedRecords list !",ObjectName);
		return false;	
	}
	if ((RemovedFeatures = new bool[con->GetFeatureCount()])==NULL)
	{
		notif->Error("[%s] -> Unable to create RemovedFeatures list !",ObjectName);
		return false;	
	}
	TreePathSize = 0;
	return true;
}
bool GenericFeatureStatistics::AddNewStatFunction(char *name,GML::ML::FeatStatComputeFunction _fnCompute)
{
	Stats	tmp;

	if (tmp.Name.Set(name)==false)
		return false;
	tmp.fnCompute = _fnCompute;
	return StatsData.PushByRef(tmp);
}
void GenericFeatureStatistics::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand)
{
	switch (threadCommand)
	{
		case THREAD_COMMAND_NONE:
			return;
		case THREAD_COMMAND_COMPUTE_FEATURES_COUNTERS:
			OnComputeFeatureCounters(thData);
			return;
		case THREAD_COMMAND_REMOVE_RECORDS:
			OnComputeRemoveIndexes(thData);
			return;
	};
}
bool GenericFeatureStatistics::OnComputeFeatureCounters(GML::Algorithm::MLThreadData &thData)
{
	UInt32								tr,gr,nrRecords,nrFeatures;
	CascadeFeatureSelectionThreadData	*obj_td = (CascadeFeatureSelectionThreadData* )thData.Context;
	
	nrRecords = con->GetRecordCount();
	nrFeatures = con->GetFeatureCount();
	// curatam datele
	for (tr=0;tr<nrFeatures;tr++)
		obj_td->Counters[tr].Reset();
	
	if (thData.ThreadID==0)
		notif->StartProcent("[%s] -> Analyzing (%d record) ",ObjectName,workingRecordsCount);
		
	for (tr=thData.ThreadID;tr<nrRecords;tr+=threadsCount)
	{		
		if (RemovedRecords[tr])
			continue;	
		if (con->GetRecord(thData.Record,tr)==false)
		{
			notif->Error("[%s] -> Unable to read record #%d",ObjectName,tr);
			return false;
		}
		for (gr=0;gr<nrFeatures;gr++)
		{
			if (RemovedFeatures[gr])
				continue;
			if (thData.Record.Features[gr]!=0)
			{
				if (thData.Record.Label>0)
				{
					obj_td->Counters[gr].CountPozitive++;
					obj_td->Counters[gr].CountTotalPozitive++;
				} else {
					obj_td->Counters[gr].CountNegative++;
					obj_td->Counters[gr].CountTotalNegative++;
				}
			} else {
				if (thData.Record.Label>0)
					obj_td->Counters[gr].CountTotalPozitive++;
				else
					obj_td->Counters[gr].CountTotalNegative++;
			}
		}
		if (callThreadComputeExtraDataFunction)
		{
			if (OnThreadComputeExtraData(tr,thData)==false)
				return false;
		}			

		if ((thData.ThreadID==0) && ((tr%1000)==0))
			notif->SetProcent(tr,nrRecords);
	}
	if (thData.ThreadID==0)
		notif->EndProcent();
	
	return true;
}
bool GenericFeatureStatistics::OnThreadComputeExtraData(UInt32 recordIndex,GML::Algorithm::MLThreadData &thData)
{
	return false;
}
bool GenericFeatureStatistics::OnComputeRemoveIndexes(GML::Algorithm::MLThreadData &thData)
{
	UInt32								tr,gr,nrRecords,nrFeatures,idFeat;
	CascadeFeatureSelectionThreadData	*obj_td = (CascadeFeatureSelectionThreadData* )thData.Context;
	
	nrRecords = con->GetRecordCount();
	nrFeatures = con->GetFeatureCount();
	obj_td->workingRecordsCount = 0;
	
	if (thData.ThreadID==0)
		notif->StartProcent("[%s] -> Creating working list ",ObjectName);
		
	for (tr=thData.ThreadID;tr<nrRecords;tr+=threadsCount)
	{
		if (con->GetRecord(thData.Record,tr)==false)
		{
			notif->Error("[%s] -> Unable to read record #%d",ObjectName,tr);
			return false;
		}
		for (gr=0;gr<TreePathSize;gr++)
		{
			idFeat = TreePath[gr] & 0x7FFFFFFF;
			if (TreePath[gr] & 0x80000000) // nesetat
			{
				if (thData.Record.Features[idFeat]!=0.0)
					break;
			} else { // setat
				if (thData.Record.Features[idFeat]==0.0)
					break;
			}
		}
		if (gr!=TreePathSize) 
		{
			RemovedRecords[tr] = true;
		} else {
			obj_td->workingRecordsCount++;
			RemovedRecords[tr] = false;
		}
		if ((thData.ThreadID==0) && ((tr%1000)==0))
			notif->SetProcent(tr,nrRecords);				
	}
	if (thData.ThreadID==0)
		notif->EndProcent();
	
	return true;
}
bool GenericFeatureStatistics::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	CascadeFeatureSelectionThreadData	*obj_td = new CascadeFeatureSelectionThreadData();
	if (obj_td==NULL)
		return false;
	if (obj_td->Counters.Create(con->GetFeatureCount(),true)==false)
		return false;
	thData.Context = obj_td;
	return true;
}
double GenericFeatureStatistics::ComputeScore(FeatureCounters &counter)
{
	FeatureInformationFromFeatureCounters(counter, finf);
	//return abs((double)counter.CountPozitive-(double)counter.CountNegative);
	return StatsData[computeMethodIndex].fnCompute(&finf);
}
void GenericFeatureStatistics::CreateWorkingList()
{
	UInt32	gr;
	// resetez lista de feateruri
	MEMSET(RemovedFeatures,0,sizeof(bool)*con->GetFeatureCount());	
	for (gr=0;gr<TreePathSize;gr++)
		RemovedFeatures[TreePath[gr] & 0x7FFFFFFF] = true;
	// resetez lista de recorduri
	if (TreePathSize==0)
	{
		workingRecordsCount = con->GetRecordCount();
		MEMSET(RemovedRecords,0,sizeof(bool)*con->GetRecordCount());
	} else {
		ExecuteParalelCommand(THREAD_COMMAND_REMOVE_RECORDS);
		workingRecordsCount = 0;
		for (gr=0;gr<threadsCount;gr++)
			workingRecordsCount += ((CascadeFeatureSelectionThreadData *)ThData[gr].Context)->workingRecordsCount;
	}
}

void GenericFeatureStatistics::ComputeScoresAndSort()
{
	UInt32				tr,gr;
	FeatureScore		fscor;
	// calculez
	ExecuteParalelCommand(THREAD_COMMAND_COMPUTE_FEATURES_COUNTERS);
	// adun
	FeatScores.DeleteAll();
	for (tr=0;tr<con->GetFeatureCount();tr++)
	{
		FeatCounters[tr].Reset();
		for (gr=0;gr<threadsCount;gr++)
			FeatCounters[tr].Add(((CascadeFeatureSelectionThreadData *)ThData[gr].Context)->Counters[tr]);
		// calculez si o valoare
		if (RemovedFeatures[tr]==false)
		{
			fscor.Index = tr;
			fscor.Score = ComputeScore(FeatCounters[tr]);
			if (FeatScores.PushByRef(fscor)==false)
			{
				notif->Error("[%s] -> Unable to add data to features score: %s",ObjectName);
				return;	
			}				
		}
	}
	// sortez
	FeatScores.Sort(cmpFunction,false);		
}

void GenericFeatureStatistics::OnExecute()
{
	switch (Command)
	{
		case COMMAND_NONE:
			notif->Error("[%s] -> Nothing to do , select another command ",ObjectName);
			break;
		case COMMAND_COMPUTE:
			OnCompute();
			break;
		default:
			notif->Error("[%s] -> Unknown command ID: %d",ObjectName,Command);
			break;
	}	
}
