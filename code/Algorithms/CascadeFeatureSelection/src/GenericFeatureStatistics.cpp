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
	computeMethod.Set("!!LIST:");
	for (tr=0;tr<funcCount;tr++)	
		computeMethod.AddFormated("%s=%d,",GML::ML::FeatStatsFunctions::GetFunctionName(tr),tr);
	if (funcCount>0)
		computeMethod.Truncate(computeMethod.Len()-1);
	computeMethod.Add("!!");
	LinkPropertyToUInt32("FeatStatMethod",computeMethodIndex,0,computeMethod.GetText());
	
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
	fnCompute = GML::ML::FeatStatsFunctions::GetFunctionPointer(computeMethodIndex);
	TreePathSize = 0;
	return true;
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
		case THREAD_COMMAND_PROCESS_RECORDS:
			OnComputeProcessedRecords(thData);
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
bool GenericFeatureStatistics::OnProcessRecord(UInt32 recordIndex,GML::Algorithm::MLThreadData &thData)
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
bool GenericFeatureStatistics::OnComputeProcessedRecords(GML::Algorithm::MLThreadData &thData)
{
	UInt32								tr,gr,nrRecords,nrFeatures,idFeat;
	CascadeFeatureSelectionThreadData	*obj_td = (CascadeFeatureSelectionThreadData* )thData.Context;
	
	nrRecords = con->GetRecordCount();
	nrFeatures = con->GetFeatureCount();
	obj_td->workingRecordsCount = 0;
	
	if (thData.ThreadID==0)
		notif->StartProcent("[%s] -> Removing records ",ObjectName);
		
	for (tr=thData.ThreadID;tr<nrRecords;tr+=threadsCount)
	{
		if ((skipRemovedRecords) && (RemovedRecords[tr]))
			continue;
		if (con->GetRecord(thData.Record,tr)==false)
		{
			notif->Error("[%s] -> Unable to read record #%d",ObjectName,tr);
			return false;
		}
		if (OnProcessRecord(tr,thData)==false)
		{
			notif->Error("[%s] -> OnProcessRecord returned false for record #%d -> exiting!",ObjectName,tr);
			return false;		
		}
		if (RemovedRecords[tr]==false)
			obj_td->workingRecordsCount++;

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
	GML::ML::FeatureInformation				finf;
	
	FeatureInformationFromFeatureCounters(counter, finf);
	if (fnCompute)
		return fnCompute(&finf);
	return 0.0;
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
void GenericFeatureStatistics::ProcessRecords(bool _skipRemovedRecords,bool clearRemoveRecordsList)
{

	if (clearRemoveRecordsList)
	{
		MEMSET(RemovedRecords,0,sizeof(bool)*con->GetRecordCount());
	}
	skipRemovedRecords = _skipRemovedRecords;
	ExecuteParalelCommand(THREAD_COMMAND_PROCESS_RECORDS);
	workingRecordsCount = 0;
	for (UInt32 gr=0;gr<threadsCount;gr++)
		workingRecordsCount += ((CascadeFeatureSelectionThreadData *)ThData[gr].Context)->workingRecordsCount;	
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
void GenericFeatureStatistics::ClearRemovedRecordsList()
{
	MEMSET(RemovedRecords,0,sizeof(bool)*con->GetRecordCount());
}
void GenericFeatureStatistics::ClearRemovedFeaturesList()
{
	MEMSET(RemovedFeatures,0,sizeof(bool)*con->GetFeatureCount());
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
