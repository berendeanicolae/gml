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
	
	LinkPropertyToUInt32("K"			,K,				3			,"K parameter from KNN algorithm");
	LinkPropertyToUInt32("Method"		,Method,		Method_UseK	,"!!LIST:UseK=0!!");
	LinkPropertyToString("ResultFileName",ResultFileName,"","The name of the file where the results will be saved !");
	//LinkPropertyToUInt32("ColumnWidth"	,columnWidth,	12			,"Sets the column width (0 for no aligniation)");
	//LinkPropertyToUInt32("ClassType",ClassType,0,"!!LIST:Positive=0,Negative,Both!!");
	//LinkPropertyToUInt32("SaveResults",SaveResults,SAVE_RESULTS_NONE,"!!LIST:None=0,Text,Parsable!!");
	//
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
	if (con->CreateMlRecord(MainRecord)==false)
	{
		notif->Error("[%s] -> Unable to create MainRecords",ObjectName);
		return false;
	}
	notif->Info("[%s] -> Listing labels ...",ObjectName);
	for (UInt32 tr=0;tr<con->GetRecordCount();tr++)
	{
		if (con->GetRecordLabel(rInfo[tr].Label,tr)==false)
		{
			notif->Error("[%s] -> Unable to read label for record #%d !",ObjectName,tr);
			return false;
		}
	}
	return true;
}
void KNNStatistics::ComputeParts(ComputePartsInfo &cpi,GML::Utils::GTFVector<RecDist>	&Dist,UInt32 start,UInt32 end,double label,bool reset)
{
	double c_label;
	if (reset)
	{
		cpi.CountDiff = cpi.CountSame = 0;
		cpi.SumDiff = cpi.SumSame = 0;
	}
	for (;start<end;start++)
	{
		c_label = rInfo[Dist[start].Index].Label;
		if (c_label==label)
		{
			cpi.CountSame++;
			cpi.SumSame+=Dist[start].Dist;
		} else {
			cpi.CountDiff++;
			cpi.SumDiff+=Dist[start].Dist;
		}
	}
}
bool KNNStatistics::ComputeDist(GML::Algorithm::MLThreadData &thData)
{
	KNNStatThData		*kst = (KNNStatThData *)thData.Context;
	UInt32				tr,gr,cntFlags,nrRecords;
	RecDist				rDist;
	ComputePartsInfo	cpi;

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
		switch (Method)
		{
			case Method_UseK:
				ComputeParts(cpi,kst->Dist,0,K,thData.Record.Label,true);
				rInfo[tr].ProcCount = (UInt16)((cpi.CountSame/(cpi.CountDiff+cpi.CountSame))*65535);
				//rInfo[tr].ProcAdd = (UInt16)((cpi.SumDiff/(cpi.SumDiff+cpi.SumSame))*65535);
				//rInfo[tr].ProcAdd = (UInt16)((cpi.AddDiff/(cpi.SumDiff+cpi.SumSame))*65535);
				break;
		}
		if (thData.ThreadID==0)
			notif->SetProcent(tr,nrRecords);
	}
	if (thData.ThreadID==0)
		notif->EndProcent();
	return true;
}
bool KNNStatistics::CreateHeaders(GML::Utils::GString &str)
{
	if (notif->SuportsObjects())
		notif->CreateObject("ResultTable","Type=List;Column_0=Hash/ID;Column_0=MinDistanceSimilar;Column_1=Label;Column_2=MaxDistancesSimilar;Column_3=MinDistanceDifferent;Column_4=MaxDistanceDifferent;Column_5=Count raport");
	if (str.Set("Hash                            |Label   |MinDistSim|MaxDistSim|MinDistDif|MaxDistDif| Count rap. |\n")==false)
		return false;
	return true;
}
bool KNNStatistics::CreateRecordInfo(UInt32 index,GML::Utils::GString &str)
{
	GML::Utils::GString		hash;
	GML::DB::RecordHash		rHash;

	if (con->GetRecordHash(rHash,index)==false)
	{
		notif->Error("[%s] -> Unable to read record hash for #%d",ObjectName,index);
		return false;
	}
	if (rHash.ToString(hash)==false)
	{
		notif->Error("[%s] -> Unable to convert record hash for #%d",ObjectName,index);
		return false;
	}
	str.Set("");
	str.AddFormatedEx("%{str,L32}|%{dbl,Z3,R8}|%{dbl,Z3,R10}|%{dbl,Z3,R10}|%{dbl,Z3,R10}|%{dbl,Z3,R10}|%{dbl,Z3,R12}|\n",
						hash.GetText(),
						rInfo[index].Label,
						rInfo[index].MinDistSimilar,
						rInfo[index].MaxDistSimilar,
						rInfo[index].MinDistNotSimilar,
						rInfo[index].MaxDistNotSimilar,
						(double)rInfo[index].ProcCount/655.35);

	return true;
}
bool KNNStatistics::SaveData()
{
	GML::Utils::File	f;
	GML::Utils::GString	temp;
	int					sz;

	if (f.Create(ResultFileName.GetText())==false)
	{
		notif->Error("[%s] -> Unable to create: %s",ObjectName,ResultFileName.GetText());
		return false;
	}
	if (CreateHeaders(temp)==false)
	{
		notif->Error("[%s] -> Unable to create table headers in %s",ObjectName,ResultFileName.GetText());
		return false;
	}
	if (f.Write(temp.GetText(),temp.Len())==false)
	{
		notif->Error("[%s] -> Unable to write table headers in %s",ObjectName,ResultFileName.GetText());
		return false;
	}
	sz = temp.Len();
	temp.Set("");
	temp.AddFormatedEx("%{c,L%%,F%%}\n",'=',sz,'=');
	if (f.Write(temp.GetText(),temp.Len())==false)
	{
		notif->Error("[%s] -> Unable to write table headers in %s",ObjectName,ResultFileName.GetText());
		return false;
	}
	// scriu fiecare element
	for (UInt32 tr=0;tr<con->GetRecordCount();tr++)
	{
		if (CreateRecordInfo(tr,temp)==false)
		{
			notif->Error("[%s] -> Unable to create record info in %s",ObjectName,ResultFileName.GetText());
			return false;
		}
		if (f.Write(temp.GetText(),temp.Len())==false)
		{
			notif->Error("[%s] -> Unable to write record info in %s",ObjectName,ResultFileName.GetText());
			return false;
		}
	}
	temp.Set("");
	temp.AddFormatedEx("%{c,L%%,F%%}\n",'=',sz,'=');
	if (f.Write(temp.GetText(),temp.Len())==false)
	{
		notif->Error("[%s] -> Unable to write end table headers in %s",ObjectName,ResultFileName.GetText());
		return false;
	}
	f.Close();
	notif->Info("[%s] -> %s created ok !",ObjectName,ResultFileName.GetText());
	return true;
}

void KNNStatistics::Compute()
{
	ExecuteParalelCommand(0);
	SaveData();
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