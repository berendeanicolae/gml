#include "CascadeFeatureSelection.h"

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
CascadeFeatureSelection::CascadeFeatureSelection()
{
	ObjectName = "CascadeFeatureSelection";

	//Add extra commands here
	SetPropertyMetaData("Command","!!LIST:None=0,Compute!!");
	
	LinkPropertyToString("ResultFile",ResultFileName,"","Name of the output file");
}
bool CascadeFeatureSelection::Init()
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
	if (BTree.Create(256)==false)
	{
		notif->Error("[%s] -> Unable to create BTree !",ObjectName);
		return false;		
	}
	TreePathSize = 0;
	return true;
}
void CascadeFeatureSelection::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand)
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
bool CascadeFeatureSelection::OnComputeFeatureCounters(GML::Algorithm::MLThreadData &thData)
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
		if ((thData.ThreadID==0) && ((tr%1000)==0))
			notif->SetProcent(tr,nrRecords);
	}
	if (thData.ThreadID==0)
		notif->EndProcent();
	
	return true;
}
bool CascadeFeatureSelection::OnComputeRemoveIndexes(GML::Algorithm::MLThreadData &thData)
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
		if (gr==TreePathSize) 
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
bool CascadeFeatureSelection::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	CascadeFeatureSelectionThreadData	*obj_td = new CascadeFeatureSelectionThreadData();
	if (obj_td==NULL)
		return false;
	if (obj_td->Counters.Create(con->GetFeatureCount(),true)==false)
		return false;
	thData.Context = obj_td;
	return true;
}
double CascadeFeatureSelection::ComputeScore(FeatureCounters &counter)
{
	return abs((double)counter.CountPozitive-(double)counter.CountNegative);
}
void CascadeFeatureSelection::CreateWorkingList()
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
bool CascadeFeatureSelection::CreatePath(UInt32 index)
{
	UInt32	*bTree = BTree.GetPtrToObject(0);
	UInt32	remove;
	
	TreePathSize = 0;
	if (index==0)
		return true;
	while (index>0)
	{
		if (TreePathSize>=MAX_PATH_DEPTH)
		{
			notif->Error("[%s] -> Depth to hi (>%d) in BTree for index:%d",ObjectName,MAX_PATH_DEPTH,index);
			return false;
		}
		remove = (((index-1) & 1)<<31);
		index = (index-1)>>1;
		TreePath[TreePathSize++] = bTree[index] | remove;		
	}
	return true;
}
void CascadeFeatureSelection::ComputeScoresAndSort()
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
void CascadeFeatureSelection::Compute()
{
	UInt32				tr,gr;
	GML::Utils::File	out;
	
	GML::Utils::GString tmp;
	
	if (out.Create(ResultFileName.GetText(),true)==false)
	{
		notif->Error("[%s] -> Unable to create: %s",ObjectName,ResultFileName.GetText());
		return;
	}
	tmp.Create(2048);
	BTree.DeleteAll();
	workingRecordsCount = con->GetRecordCount();

	while (true)
	{
		// calculez counterele
	
		if (CreatePath(BTree.Len())==false)
			return;
		CreateWorkingList();
		// afisez working list
		tmp.SetFormated("Working List (%d):",TreePathSize);
		for (tr=0;tr<TreePathSize;tr++)
		{
			if (TreePath[tr] & 0x80000000)
				tmp.Add("-");
			else
				tmp.Add("+");
			tmp.AddFormated("%d",TreePath[tr] & 0x7FFFFFFF);
			tmp.Add(",");
		}
		notif->Info("[%s] -> %s",ObjectName,tmp.GetText());

		ComputeScoresAndSort();
		if (FeatScores.Len()==0)
		{
			notif->Info("[%s] -> No more features left ... ending",ObjectName);
			break;
		}
		
		
		featToRemove = FeatScores[0].Index;
		BTree.PushByRef(FeatScores[0].Index);
		
		// afisez
		tmp.SetFormated("%d|Scor:%lf|Poz:%d|Neg:%d|TotalPoz:%d|TotalNeg:%d\n",
						featToRemove,
						FeatScores[0].Score,
						FeatCounters[featToRemove].CountPozitive,
						FeatCounters[featToRemove].CountNegative,
						FeatCounters[featToRemove].CountTotalPozitive,
						FeatCounters[featToRemove].CountTotalNegative);
		
		out.Write(tmp.GetText(),tmp.Len());
	}	
	out.Close();
}
void CascadeFeatureSelection::OnExecute()
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
