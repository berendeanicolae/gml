#include "Centroid.h"

#define FIND_CENTROID	0


Centroid::Centroid()
{
	ObjectName = "Centroid";

	SetPropertyMetaData("Command","!!LIST:None=0,Train,Test!!");

		
}
void Centroid::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand)
{
	switch (threadCommand)
	{
		case FIND_CENTROID:
			FindCentroid(thData,indexesPozitive,indexesPozitive,indexesNegative);
			return;
	}
}
bool Centroid::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	CentroidThreadData	*ctd = new CentroidThreadData();
	if (ctd==NULL)
		return false;

	if (con->CreateMlRecord(ctd->SecRec)==false)
		return false;

	thData.Context = ctd;
	return true;
}
bool Centroid::CreatePozitiveAndNegativeIndexes()
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
bool Centroid::Init()
{
	UInt32				tr;
	CentroidDistances	cd;

	if (InitConnections()==false)
		return false;
	if (CreatePozitiveAndNegativeIndexes()==false)
		return false;
	if (InitThreads()==false)
		return false;
	if (distInfo.Create(con->GetRecordCount())==false)
	{
		notif->Error("[%s] -> Unable to alloc %d elements for distInfo",ObjectName,con->GetRecordCount());
		return false;
	}
	for (tr=0;tr<con->GetRecordCount();tr++)
	{
		cd.index = tr;
		cd.corectelyClasify = 0;
		cd.ClosestDifferentDistance = 0.0;
		cd.MostDistantSimilarDistance = 0.0;
		if (distInfo.PushByRef(cd)==false)
		{
			notif->Error("[%s] -> Unable to add elements for distInfo",ObjectName);
			return false;
		}
	}
	return true;
}
bool Centroid::FindCentroid(GML::Algorithm::MLThreadData &thData,GML::Utils::Indexes &indexWork,GML::Utils::Indexes &indexPoz,GML::Utils::Indexes &indexNeg)
{
	UInt32					tr,gr,featuresCount,sameClassCount,diffClassCount,workCount,idxWork,idx,countClasified;
	CentroidThreadData*		dt = (CentroidThreadData *)thData.Context;
	GML::Utils::Indexes		*indexSameClass,*indexDiffClass;
	CentroidDistances*		cd = distInfo.GetVector();
	double					dist,maxDiffDist,centroidRay;
	

	featuresCount = con->GetFeatureCount();
	workCount = indexWork.Len();

	if (thData.ThreadID==0)
		notif->StartProcent("[%s] -> Computing ... ",ObjectName);
	for (tr=thData.ThreadID;tr<workCount;tr+=threadsCount)
	{
		idxWork = indexWork.Get(tr);
		if (con->GetRecord(thData.Record,idxWork)==false)
		{
			notif->Error("[%s] -> Unable to read record #%d",ObjectName,idxWork);
			return false;
		}
		// caut cel mai apropiat de o clasa diferita
		if (thData.Record.Label == 1)
		{
			indexSameClass = &indexPoz;
			indexDiffClass = &indexNeg;
		} else {
			indexSameClass = &indexNeg;
			indexDiffClass = &indexPoz;
		}
		sameClassCount = indexSameClass->Len();
		diffClassCount = indexDiffClass->Len();
		
		// caut din clasa diferita cel mai apropiat
		maxDiffDist = 10000000000;
		for (gr=0;gr<diffClassCount;gr++)
		{
			idx = indexDiffClass->Get(tr);
			if (con->GetRecord(dt->SecRec,idx)==false)
			{
				notif->Error("[%s] -> Unable to read record #%d",ObjectName,idx);
				return false;
			}
			dist = GML::ML::VectorOp::PointToPointDistance(thData.Record.Features,dt->SecRec.Features,featuresCount);
			if (dist<maxDiffDist)
				maxDiffDist = dist;
		}

		// numar sa vad dintre cele similare cate sunt apropiate de el
		countClasified = 0;
		centroidRay = 0.0;
		for (gr=0;gr<sameClassCount;gr++)
		{
			idx = indexSameClass->Get(tr);
			if (con->GetRecord(dt->SecRec,idx)==false)
			{
				notif->Error("[%s] -> Unable to read record #%d",ObjectName,idx);
				return false;
			}
			dist = GML::ML::VectorOp::PointToPointDistance(thData.Record.Features,dt->SecRec.Features,featuresCount);
			if (dist<maxDiffDist)
			{
				countClasified++;
				if (dist>centroidRay)
					centroidRay = dist;
			}
			cd[tr].corectelyClasify = countClasified;
			cd[tr].ClosestDifferentDistance = maxDiffDist;
			cd[tr].MostDistantSimilarDistance = centroidRay;
		}

		// salvez cate ar putea clasifica
		if (thData.ThreadID==0)
			notif->SetProcent(tr,workCount);
	}
	if (thData.ThreadID==0)
		notif->EndProcent();
	return true;
}
void Centroid::OnExecute()
{
	if (Command==1)	//Train
	{
		ExecuteParalelCommand(FIND_CENTROID);
		return;
	}
	notif->Error("[%s] -> Unknown command ID: %d",ObjectName,Command);
}