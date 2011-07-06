#include "BinaryDecisionTree.h"

#define COMPUTE_FEATURES_STATS		1

void BDTThreadData::Clear()
{
	totalNegative = 0;
	totalPozitive = 0;
	for (UInt32 tr=0;tr<FeaturesCount.Len();tr++)
	{
		FeaturesCount[tr].NegativeCount = 0;
		FeaturesCount[tr].PozitiveCount = 0;
		FeaturesCount[tr].Score = 0.0;
		FeaturesCount[tr].Index = tr;
	}
}
void BDTThreadData::Add(BDTThreadData &obj)
{
	totalNegative+=obj.totalNegative;
	totalPozitive+=obj.totalPozitive;
	for (UInt32 tr=0;tr<FeaturesCount.Len();tr++)
	{
		FeaturesCount[tr].NegativeCount += obj.FeaturesCount[tr].NegativeCount;
		FeaturesCount[tr].PozitiveCount += obj.FeaturesCount[tr].PozitiveCount;
	}
}
bool BDTThreadData::Create(UInt32 nrFeatures)
{
	IndexRecords = NULL;
	if (FeaturesCount.Create(nrFeatures,true)==false)
		return false;
	Clear();
	return true;
}
int FeatureCountCompare(FeaturesInfo &f1,FeaturesInfo &f2)
{
	if (f1.Score>f2.Score)
		return 1;
	if (f1.Score<f2.Score)
		return -1;
	return 0;
}
//=============================================================================


BinaryDecisionTree::BinaryDecisionTree()
{
	ObjectName = "BinaryDecisionTree";

	SetPropertyMetaData("Command","!!LIST:None=0,Train!!");
	
}

void BinaryDecisionTree::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand)
{
	switch (threadCommand)
	{
		case COMPUTE_FEATURES_STATS:
			ComputeFeatureStats(thData);
			return;
	};
}
bool BinaryDecisionTree::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	BDTThreadData	*dth = new BDTThreadData();
	
	if (dth==NULL)
		return false;
	if (dth->Create(con->GetFeatureCount())==false)	
		return false;
		
	dth->IndexRecords = NULL;
	thData.Context = dth;
	return true;
}

bool BinaryDecisionTree::Init()
{
	if (InitConnections()==false)
		return false;
	if (InitThreads()==false)
		return false;
	return true;
}
bool BinaryDecisionTree::ComputeFeatureStats(GML::Algorithm::MLThreadData &thData)
{
	UInt32			tr,gr,nrFeat,nrRec;
	BDTThreadData	*td = (BDTThreadData *)thData.Context;

	nrFeat = con->GetFeatureCount();
	nrRec = td->IndexRecords->Len();

	// clear the data
	td->Clear();
	// citesc datele asociate range-ului
	if (thData.ThreadID==0)
		notif->StartProcent("[%s] -> Analizing features ... ",ObjectName);
	for (tr=thData.ThreadID;(tr<nrRec) && (StopAlgorithm==false);tr+=threadsCount)
	{
		if (con->GetRecord(thData.Record,(*td->IndexRecords)[tr])==false)
		{
			notif->Error("[%s] -> Unable to read record %d",ObjectName,(*td->IndexRecords)[tr]);
			return false;
		}
		if (thData.Record.Label==1)
			td->totalPozitive++;
		else
			td->totalNegative++;
		for (gr=0;(gr<nrFeat) && (StopAlgorithm==false);gr++)
		{
			if (thData.Record.Features[gr]!=0)
			{
				if (thData.Record.Label==1)
					td->FeaturesCount[gr].PozitiveCount++;
				else
					td->FeaturesCount[gr].NegativeCount++;
			}
		}
		if (thData.ThreadID==0)
			notif->SetProcent(tr,nrRec);
	}


	if (thData.ThreadID==0)
		notif->EndProcent();
	return true;
}
bool BinaryDecisionTree::ComputeFeaturesStatistics(GML::Utils::GTFVector<UInt32> *Indexes,BDTThreadData	&all)
{
	
	BDTThreadData	*curent;
	UInt32			tr;

	for (tr=0;tr<threadsCount;tr++)
	{
		curent = (BDTThreadData *)ThData[tr].Context;
		curent->IndexRecords = Indexes;
	}
	if (all.Create(con->GetFeatureCount())==false)
	{
		notif->Error("[%s] -> Unable to create feature stats buffer !");
		return false;
	}
	ExecuteParalelCommand(COMPUTE_FEATURES_STATS);
	for (UInt32 tr=0;tr<threadsCount;tr++)
	{
		curent = (BDTThreadData *)ThData[tr].Context;
		all.Add(*curent);
	}
	// in all am toate
	return true;
}
void BinaryDecisionTree::ComputeScore(BDTThreadData	&all,double (*fnComputeScore)(FeaturesInfo &fi,UInt32 totalPozitive,UInt32 totalNegative))
{
	UInt32	tr;

	for (tr=0;tr<all.FeaturesCount.Len();tr++)
	{
		all.FeaturesCount[tr].Score = fnComputeScore(all.FeaturesCount[tr],all.totalPozitive,all.totalNegative);
	}
	all.FeaturesCount.Sort(FeatureCountCompare);
}

void BinaryDecisionTree::OnExecute()
{
	if (Command==1)	//Train
	{
		
		return;
	}
	notif->Error("[%s] -> Unknown command ID: %d",ObjectName,Command);
}