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

double log2(double x)
{
	if (x==0) return 0;
	return log10(x) / (log10((double)2));
}
double entropy(double v_true,double v_false)
{
	if ((v_true+v_false)==0) return 0;
	double v1=((double)v_true)/((double)(v_true+v_false));
	double v2=((double)v_false)/((double)(v_true+v_false));
	return -(v1*log2(v1)) - (v2*log2(v2));
}

double Compute_InformationGain(FeaturesInfo &fi, UInt32 totalPozitive, UInt32 totalNegative)
{
	double e, e1, e2;
	double total = totalPozitive+totalNegative;
	e = entropy(fi.PozitiveCount + (totalNegative-fi.NegativeCount), fi.NegativeCount+ (totalNegative-fi.NegativeCount));
	e1 = entropy(fi.PozitiveCount, fi.NegativeCount);
	e2 = entropy(fi.PozitiveCount, (totalNegative-fi.NegativeCount));

	return e - ((double)(fi.PozitiveCount + fi.NegativeCount) / total) * e1 - ((double)((totalPozitive-fi.PozitiveCount) + (totalNegative-fi.NegativeCount)) / total) * e2;
}
double Compute_Sum(FeaturesInfo &fi, UInt32 totalPozitive, UInt32 totalNegative)
{
	return fi.PozitiveCount+fi.NegativeCount;
}
double Compute_FAQ(FeaturesInfo &fi, UInt32 totalPozitive, UInt32 totalNegative)
{
	return abs((1.0/((fi.PozitiveCount+1.0)/totalPozitive-0.5))*(1.0/((fi.NegativeCount+1.0)/totalNegative-0.5)));
	//return ((fi.PozitiveCount+1.0)/totalPozitive)*((fi.NegativeCount+1.0)/totalNegative);
}
//=============================================================================
BinaryDecisionTree::BinaryDecisionTree()
{
	ObjectName = "BinaryDecisionTree";

	SetPropertyMetaData("Command","!!LIST:None=0,Train,CustomFeatSplit!!");
	LinkPropertyToString("HashBaseFileName", HashBaseFileName, "", "BaseName for the file the hashes will be saved in");
	LinkPropertyToString("CustomFeatName", CustomFeatName, "", "FeatureName to split records by (name or #index)");		
	LinkPropertyToUInt32("ComputeScoreMethod",ComputeScoreMethod,0,"!!LIST:InformationGain=0,SumPosCountNegCount,FAQ!!");
	AddHashSaveProperties();	
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
	if (con->CreateMlRecord(MainRecord)==false)
	{
		notif->Error("[%s] -> Unable to create the main records !",ObjectName);
		return false;
	}
	if (RecordsStatus.Create(con->GetRecordCount(),true)==false)
	{
		notif->Error("[%s] -> Unable to create vector information for records !",ObjectName);
		return false;
	}
	return true;
}
bool BinaryDecisionTree::ComputeFeatureStats(GML::Algorithm::MLThreadData &thData)
{
	UInt32			tr,gr,nrFeat,nrRec,count=0;
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
		{
			count++;
			if (count>1000)
			{
				notif->SetProcent(tr,nrRec);
				count=0;
			}
		}
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
	all.FeaturesCount.Sort(FeatureCountCompare, false);
}
bool BinaryDecisionTree::SaveHashesForFeature(char *fileName,GML::Utils::GTFVector<UInt32> *Indexes,UInt32 featIndex,bool featureValue)
{
	UInt32	idx,tr;

	for (tr=0;tr<con->GetRecordCount();tr++)
		RecordsStatus[tr]=0;
	for (tr=0;tr<Indexes->Len();tr++)
	{
		idx = (*Indexes)[tr];
		if (con->GetRecord(MainRecord,idx)==false)
		{
			notif->Error("[%s] -> Unable to read record %d",ObjectName,idx);
			return false;
		}
		if ((MainRecord.Features[featIndex] == 1) && (featureValue))
			RecordsStatus[idx] = 1;
		if ((MainRecord.Features[featIndex] != 1) && (!featureValue))
			RecordsStatus[idx] = 1;
	}
	return SaveHashResult(fileName,HashFileType,RecordsStatus);	
}
bool BinaryDecisionTree::CreateIndexes(GML::Utils::GTFVector<UInt32> *Indexes)
{
	UInt32	recordsCount;	

	recordsCount = con->GetRecordCount();
	if (Indexes->Create(recordsCount)==false)
	{
		notif->Error("[%s] -> Unable to alloc %d entries for indexes",ObjectName, recordsCount);
		return false;
	}
	for (int i=0; i<recordsCount; i++)
		Indexes->Push(i);
	return true;
}

void BinaryDecisionTree::PerformComputeScore(BDTThreadData	&all)
{
	switch(ComputeScoreMethod)
	{
		case COMPUTE_SCORE_IG :
			ComputeScore(all, Compute_InformationGain);		
			break;
		case COMPUTE_SCORE_SUM:
			ComputeScore(all, Compute_Sum);		
			break;
		case COMPUTE_SCORE_FREQ:
			ComputeScore(all, Compute_FAQ);		
			break;
	}		
}
bool BinaryDecisionTree::PerformTrain()
{
	GML::Utils::GTFVector<UInt32>	indexes;	
	BDTThreadData					bdtthreadData;
	GML::Utils::GString				fileName;
	GML::Utils::GString				featName;
	
	if (CreateIndexes(&indexes) == false)
	{
			notif->Error("[%s] -> Something went wrong while creating indexes!!!",ObjectName);
			return false;
	}
	ComputeFeaturesStatistics(&indexes, bdtthreadData);
	PerformComputeScore(bdtthreadData);

	for (int tr=0;tr<10;tr++)
	{
		if (con->GetFeatureName(featName, bdtthreadData.FeaturesCount[tr].Index) == false)
		{
			notif->Error("[%s] -> Unable to get feature name for feature with index: %d",ObjectName, bdtthreadData.FeaturesCount[0].Index);
			return false;
		}
		fileName.Set("");
		fileName.AddFormatedEx("%{str,L40} Index=%{uint32,dec,R8}  Score=%{double,Z4,R10} PC:%{uint32,dec,R8} TP:%{uint32,dec,R8} NC:%{uint32,dec,R8} TN:%{uint32,dec,R8}",featName.GetText(),bdtthreadData.FeaturesCount[tr].Index,bdtthreadData.FeaturesCount[tr].Score,bdtthreadData.FeaturesCount[tr].PozitiveCount, bdtthreadData.totalPozitive, bdtthreadData.FeaturesCount[tr].NegativeCount, bdtthreadData.totalNegative);
		notif->Info("%s",fileName.GetText());
	}

	if (con->GetFeatureName(featName, bdtthreadData.FeaturesCount[0].Index) == false)
	{
		notif->Error("[%s] -> Unable to get feature name for feature with index: %d",ObjectName, bdtthreadData.FeaturesCount[0].Index);
		return false;
	}		
	fileName.SetFormated("%s[%s][%d]_1",HashBaseFileName.GetText(),featName.GetText(), bdtthreadData.FeaturesCount[0].Index);
	SaveHashesForFeature(fileName.GetText(), &indexes, bdtthreadData.FeaturesCount[0].Index, true);	
	fileName.SetFormated("%s[%s][%d]_0",HashBaseFileName.GetText(),featName.GetText(), bdtthreadData.FeaturesCount[0].Index);
	SaveHashesForFeature(fileName.GetText(), &indexes, bdtthreadData.FeaturesCount[0].Index, false);
	return true;
}
bool BinaryDecisionTree::PerformCustomFeatureSplit()
{
	GML::Utils::GTFVector<UInt32>	indexes;
	UInt32							featIndex;
	GML::Utils::GString				fileName;
	GML::Utils::GString				featName;

	if (CustomFeatName.Equals(""))
	{
		notif->Error("[%s] -> Must provide CustomFeatureName!!!",ObjectName);
		return false;
	}
	if (CreateIndexes(&indexes) == false)
	{
			notif->Error("[%s] -> Something went wrong while creating indexes!!!",ObjectName);
			return false;
	}	
	if (CustomFeatName.StartsWith("#"))	// avem indexul feature`ului
	{
		if (!CustomFeatName.Replace("#", ""));
		{
		//	notif->Error("[%s] -> Failed to extract feature`s index!!!",ObjectName);
//			return false;
		}
		if (!CustomFeatName.ConvertToUInt32(&featIndex, 10)) 			
		{
			notif->Error("[%s] -> Failed to extract feature`s index!!!",ObjectName);
			return false;
		}
		if (featIndex<0 || featIndex>=con->GetFeatureCount())
		{
			notif->Error("[%s] -> wrong feature`s index. Must be inside [0,%d]!!!",ObjectName, con->GetFeatureCount());
			return false;
		}
		if (con->GetFeatureName(featName, featIndex) == false)
		{
			notif->Error("[%s] -> Unable to get feature name for feature with index: %d",ObjectName, featIndex);
			return false;
		}		
		fileName.SetFormated("%s[%s][%d]_1",HashBaseFileName.GetText(),featName.GetText(), featIndex);
		SaveHashesForFeature(fileName.GetText(), &indexes, featIndex, true);	
		fileName.SetFormated("%s[%s][%d]_0",HashBaseFileName.GetText(),featName.GetText(), featIndex);
		SaveHashesForFeature(fileName.GetText(), &indexes, featIndex, false);
		return true;
	} else 
	{
		for (featIndex=0; featIndex<con->GetFeatureCount(); featIndex++)
		{
			if (con->GetFeatureName(featName, featIndex) == false)
			{
				notif->Error("[%s] -> Unable to get feature name for feature with index: %d",ObjectName, featIndex);
				return false;
			}	
			if (featName.Equals(CustomFeatName))	// am gasit indexul pentru feature`ul dorit
			{
				if (featIndex<0 || featIndex>=con->GetFeatureCount())
				{
					notif->Error("[%s] -> wrong feature`s index. Must be inside [0,%d]!!!",ObjectName, con->GetFeatureCount());
					return false;
				}				
				fileName.SetFormated("%s[%s][%d]_1",HashBaseFileName.GetText(),featName.GetText(), featIndex);
				SaveHashesForFeature(fileName.GetText(), &indexes, featIndex, true);	
				fileName.SetFormated("%s[%s][%d]_0",HashBaseFileName.GetText(),featName.GetText(), featIndex);
				SaveHashesForFeature(fileName.GetText(), &indexes, featIndex, false);
				return true;
			}
		}
		notif->Error("[%s] -> Wrong FeatureName; [%s] couldn`t be found...",ObjectName, CustomFeatName.GetText());
		return false;
	}

	return true;
}

void BinaryDecisionTree::OnExecute()
{
	switch (Command)
	{
		case COMMAND_NONE:
			notif->Info("[%s] -> Nothing to do ... ",ObjectName);
			return;
		case COMMAND_TRAIN:
			PerformTrain();
			return;
		case COMMAD_SPLIT_CUSTOM_FEAT:
			PerformCustomFeatureSplit();
			return;
	};
	notif->Error("[%s] -> Unkwnown command ID : %d",ObjectName,Command);
}