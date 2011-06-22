#include "MapTemplate.h"

//================================================================================


MapTemplate::MapTemplate()
{
	ObjectName = "MapTemplate";

	SetPropertyMetaData("Command","!!LIST:None=0,Compute!!");
	
	

	//LinkPropertyToUInt32("ClassType",ClassType,0,"!!LIST:Positive=0,Negative,Both!!");
	//LinkPropertyToUInt32("SaveResults",SaveResults,SAVE_RESULTS_NONE,"!!LIST:None=0,Text,Parsable!!");
	//LinkPropertyToString("ResultFileName",ResultFileName,"","The name of the file where the results will be saved !");
	//LinkPropertyToString("MapTemplateFileName",MapTemplateFileName,"","The name of the file that will be use as a pattern for centroid saving !");
	//LinkPropertyToBool  ("SortResults",SortResults,false,"Specify if the results should be sorted before saving");
	//LinkPropertyToUInt32("MinimPositiveElements",minPositiveElements,0,"Specify the minimum number of positive elemens in a centroid");
	//LinkPropertyToUInt32("MinimNegativeElements",minNegativeElements,0,"Specify the minimum number of negative elemens in a centroid");

	//LinkPropertyToString("MapTemplatesFileList"		,MapTemplatesFileList		,"","A list of weight files to be loaded separated by a comma.");
	//LinkPropertyToString("MapTemplatesPath"			,MapTemplatesPath			,"*.txt","The path where the weigh files are");
	//LinkPropertyToString("RayPropertyName"			,RayPropertyName		,"ray","The name of the property that contains the ray of the centroid. It has to be a numeric property.");
	//LinkPropertyToString("VotePropertyName"			,VotePropertyName		,"vote","The name of the property that contains the vote of the centroid. It has to be a numeric property.");
	//LinkPropertyToUInt32("MapTemplatesLoadingMethod"	,MapTemplatesLoadingMethod	,0,"!!LIST:FromList=0,FromPath!!");
	

		
}


void MapTemplate::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand)
{
	switch (threadCommand)
	{
		case OP_AND_2:
		case OP_XOR_2:
			Compute2LevelOps(thData,threadCommand);
			return;
	};
}
bool MapTemplate::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	GML::Utils::GTFVector<MapTemplateOp>	*op = new GML::Utils::GTFVector<MapTemplateOp>();

	if (op->Create(con->GetFeatureCount())==0)
		return false;

	thData.Context = op;
	return true;
}
bool MapTemplate::Init()
{
	if (InitConnections()==false)
		return false;
	if (InitThreads()==false)
		return false;

	return true;
}
bool MapTemplate::Compute2LevelOps(GML::Algorithm::MLThreadData &thData,unsigned int op)
{
	GML::Utils::GTFVector<MapTemplateOp>	*listOp = (GML::Utils::GTFVector<MapTemplateOp> *)thData.Context;
	UInt32									tr,gr,cntFlags,nrRecords;
	UInt32									indexRec;
	UInt32									result;
	MapTemplateOp							mtop;

	cntFlags = con->GetFeatureCount();
	nrRecords = con->GetRecordCount();

	nrRecords = 500;

	mtop.Op = op;

	if (thData.ThreadID==0)
		notif->StartProcent("[%s] -> Computing ... ",ObjectName);

	for (tr=thData.ThreadID;tr<cntFlags;tr+=threadsCount)
	{
		for (gr=tr+1;gr<cntFlags;gr++)
		{
			mtop.countNegative = mtop.countPositive = 0;
			// citesc fiecare record
			for (indexRec = 0;indexRec<nrRecords;indexRec++)
			{
				if (con->GetRecord(thData.Record,indexRec)==false)
				{
					notif->Error("[%s] -> Unable to read record #%d",ObjectName,indexRec);
					return false;
				}
				// contorizez operatia
				switch (op)
				{
					case OP_AND_2:
						result = (UInt32)(thData.Record.Features[tr]) & (UInt32)(thData.Record.Features[gr]);
						break;
					case OP_XOR_2:
						result = (UInt32)(thData.Record.Features[tr]) ^ (UInt32)(thData.Record.Features[gr]);
						break;
					default:
						result=0;
						break;
				}
				if (result!=0)
				{
					if (thData.Record.Label==1)
						mtop.countPositive++;
					else
						mtop.countNegative++;
				}
			}
			// adaug la lista
			if (mtop.countNegative+mtop.countPositive>0)
			{
				mtop.Data.FeatureIndex[0] = tr;
				mtop.Data.FeatureIndex[1] = gr;
				if (listOp->PushByRef(mtop)==false)
				{
					notif->Error("[%s] -> Unable to add map method to list ",ObjectName);
					return false;
				}
			}
		}
		if (thData.ThreadID==0)
			notif->SetProcent(tr,cntFlags);
	}
	if (thData.ThreadID==0)
		notif->EndProcent();
	return true;
}

void MapTemplate::Compute()
{
	UInt32	tr,count=0;
	GML::Utils::GTFVector<MapTemplateOp>	*listOp;

	ExecuteParalelCommand(OP_XOR_2);
	for (tr=0;tr<threadsCount;tr++)
	{
		listOp = (GML::Utils::GTFVector<MapTemplateOp> *)ThData[tr].Context;
		count+= listOp->Len();
	}
	notif->Info("Result = %d",count);
}

void MapTemplate::OnExecute()
{
	if (Command==1)	//Compute
	{
		Compute();
		return;
	}
	notif->Error("[%s] -> Unknown command ID: %d",ObjectName,Command);
}