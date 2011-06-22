#include "KNNStatistics.h"

int RecDistCompare(RecDist &r1,RecDist &r2)
{
	if (r1.Dist>r2.Dist)
		return 1;
	if (r1.Dist<r2.Dist)
		return -1;
	return 0;
}
//================================================================================


KNNStatistics::KNNStatistics()
{
	ObjectName = "KNNStatistics";

	SetPropertyMetaData("Command","!!LIST:None=0,Compute!!");
	
	

	//LinkPropertyToUInt32("ClassType",ClassType,0,"!!LIST:Positive=0,Negative,Both!!");
	//LinkPropertyToUInt32("SaveResults",SaveResults,SAVE_RESULTS_NONE,"!!LIST:None=0,Text,Parsable!!");
	//LinkPropertyToString("ResultFileName",ResultFileName,"","The name of the file where the results will be saved !");
	//LinkPropertyToString("KNNStatisticsFileName",KNNStatisticsFileName,"","The name of the file that will be use as a pattern for centroid saving !");
	//LinkPropertyToBool  ("SortResults",SortResults,false,"Specify if the results should be sorted before saving");
	//LinkPropertyToUInt32("MinimPositiveElements",minPositiveElements,0,"Specify the minimum number of positive elemens in a centroid");
	//LinkPropertyToUInt32("MinimNegativeElements",minNegativeElements,0,"Specify the minimum number of negative elemens in a centroid");

	//LinkPropertyToString("KNNStatisticssFileList"		,KNNStatisticssFileList		,"","A list of weight files to be loaded separated by a comma.");
	//LinkPropertyToString("KNNStatisticssPath"			,KNNStatisticssPath			,"*.txt","The path where the weigh files are");
	//LinkPropertyToString("RayPropertyName"			,RayPropertyName		,"ray","The name of the property that contains the ray of the centroid. It has to be a numeric property.");
	//LinkPropertyToString("VotePropertyName"			,VotePropertyName		,"vote","The name of the property that contains the vote of the centroid. It has to be a numeric property.");
	//LinkPropertyToUInt32("KNNStatisticssLoadingMethod"	,KNNStatisticssLoadingMethod	,0,"!!LIST:FromList=0,FromPath!!");
	

		
}


void KNNStatistics::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand)
{
	ComputeDist(thData);
}
bool KNNStatistics::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	KNNStatThData	*kst = new KNNStatThData();
	if (kst==NULL)
		return false;
	if (kst->Dist.Create(con->GetRecordCount())==false)
		return false;
	if (con->CreateMlRecord(kst->SecRec)==false)
		return false;
	thData.Context = kst;
	return true;
}
bool KNNStatistics::Init()
{
	if (InitConnections()==false)
		return false;
	if (InitThreads()==false)
		return false;
	if ((rInfo = new RecInfo[con->GetRecordCount()])==false)
	{
		notif->Error("[%s] -> Unable to create RecInfo object with %d records !",ObjectName,con->GetRecordCount());
		return false;
	}
	return true;
}
bool KNNStatistics::ComputeDist(GML::Algorithm::MLThreadData &thData)
{
	KNNStatThData	*kst = (KNNStatThData *)thData.Context;
	UInt32			tr,gr,cntFlags,nrRecords;
	RecDist			rDist;

	cntFlags = con->GetFeatureCount();
	nrRecords = con->GetRecordCount();


	if (thData.ThreadID==0)
		notif->StartProcent("[%s] -> Computing ... ",ObjectName);

	for (tr=thData.ThreadID;tr<nrRecords;tr+=threadsCount)
	{
		if (con->GetRecord(thData.Record,tr)==false)
		{
			notif->Error("[%s] -> Unable to read record #%d",ObjectName,tr);
			return false;
		}
		kst->Dist.DeleteAll();
		for (gr=0;gr<nrRecords;gr++)
		{
			if (gr==tr)
				continue;
			if (con->GetRecord(kst->SecRec,gr)==false)
			{
				notif->Error("[%s] -> Unable to read record #%d",ObjectName,gr);
				return false;
			}
			rDist.Index = gr;
			rDist.Dist = GML::ML::VectorOp::EuclideanDistanceSquared(thData.Record.Features,kst->SecRec.Features,cntFlags);
			if (kst->Dist.PushByRef(rDist)==false)
			{
				notif->Error("[%s] -> Unable to add record #%d",ObjectName,gr);
				return false;
			}
		}
		// sortez
		kst->Dist.Sort(RecDistCompare);
		// fac calculele
		if (thData.ThreadID==0)
			notif->SetProcent(tr,nrRecords);
	}
	if (thData.ThreadID==0)
		notif->EndProcent();
	return true;
}

void KNNStatistics::Compute()
{
	ExecuteParalelCommand(0);
}

void KNNStatistics::OnExecute()
{
	if (Command==1)	//Compute
	{
		Compute();
		return;
	}
	notif->Error("[%s] -> Unknown command ID: %d",ObjectName,Command);
}