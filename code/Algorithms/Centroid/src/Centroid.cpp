#include "Centroid.h"

#define FIND_CENTROID_POZ	0
#define FIND_CENTROID_NEG	1

int CentroidDistanceCompareFunction(CentroidDistances &c1,CentroidDistances &c2)
{
	if (c1.corectelyClasify>c2.corectelyClasify)
		return 1;
	if (c1.corectelyClasify<c2.corectelyClasify)
		return -1;
	return 0;
}
//========================================================================================================================

Centroid::Centroid()
{
	ObjectName = "Centroid";

	SetPropertyMetaData("Command","!!LIST:None=0,Compute,Test!!");
	
	

	LinkPropertyToUInt32("ClassType",ClassType,0,"!!LIST:Positive=0,Negative,Both!!");
	LinkPropertyToUInt32("SaveResults",SaveResults,SAVE_RESULTS_NONE,"!!LIST:None=0,Text,Parsable!!");
	LinkPropertyToString("ResultFileName",ResultFileName,"","The name of the file where the results will be saved !");
	LinkPropertyToString("CentroidFileName",CentroidFileName,"","The name of the file that will be use as a pattern for centroid saving !");
	LinkPropertyToBool  ("SortResults",SortResults,false,"Specify if the results should be sorted before saving");
	LinkPropertyToUInt32("MinimPositiveElements",minPositiveElements,0,"Specify the minimum number of positive elemens in a centroid");
	LinkPropertyToUInt32("MinimNegativeElements",minNegativeElements,0,"Specify the minimum number of negative elemens in a centroid");
		
}
void Centroid::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand)
{
	switch (threadCommand)
	{
		case FIND_CENTROID_POZ:
			FindCentroid(thData,indexesPozitive,indexesPozitive,indexesNegative);
			return;
		case FIND_CENTROID_NEG:
			FindCentroid(thData,indexesNegative,indexesPozitive,indexesNegative);
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
	if (con->CreateMlRecord(MainRecord)==false)
	{
		notif->Error("[%s] -> Unable to create MainRecord",ObjectName);
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
bool Centroid::BuildHeaders(GML::Utils::GString &str)
{
if (SaveResults==SAVE_RESULTS_PARSABLE)
	{		
		if (!str.Set("Hash|Index|Class|Elements|MinRay|MaxRay|"))
			return false;
	} else {
		if (!str.Set("Hash                            | Index|C|Elements|   MinRay|   MaxRay|"))
			return false;	
	}
	return true;
}
bool Centroid::BuildLineRecord(CentroidDistances *cd,GML::Utils::GString &str)
{
	double					label;
	char					cLabel;
	GML::Utils::GString		tmp;
	GML::DB::RecordHash		rHash;

	if (con->GetRecordHash(rHash,cd->index)==false)
	{
		str.Set("");
	} else {
		if (rHash.ToString(str)==false)
			return false;
	}
	if (con->GetRecordLabel(label,cd->index)==false)
		return false;
	
	if (SaveResults==SAVE_RESULTS_PARSABLE)
	{		
		if (label==1)
			cLabel='+';
		else
			cLabel='-';
		if (!str.AddFormated("|%d|%c|%d|%lf|%lf",cd->index,cLabel,cd->corectelyClasify,cd->MostDistantSimilarDistance,cd->ClosestDifferentDistance))
			return false;
	} else {
		while (str.Len()<32)
			if (!str.AddChar(' '))
				return false;

		if (str.AddFormated("|%6d|",cd->index)==false)
			return false;
		if (label==1)
		{
			if (str.Add("+|")==false)
				return false;
		} else {
			if (str.Add("-|")==false)
				return false;
		}
		if (str.AddFormated("%8d|",cd->corectelyClasify)==false)
			return false;
		if (tmp.SetFormated("%.3lf|",cd->MostDistantSimilarDistance)==false)
			return false;
		while (tmp.Len()<10)
			if (tmp.Insert(" ",0)==false)
				return false;
		if (str.Add(&tmp)==false)
			return false;
		if (tmp.SetFormated("%.3lf|",cd->ClosestDifferentDistance)==false)
			return false;
		while (tmp.Len()<10)
			if (tmp.Insert(" ",0)==false)
				return false;
		if (str.Add(&tmp)==false)
			return false;		
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
			idx = indexDiffClass->Get(gr);
			if (con->GetRecord(dt->SecRec,idx)==false)
			{
				notif->Error("[%s] -> Unable to read record #%d",ObjectName,idx);
				return false;
			}
			dist = GML::ML::VectorOp::PointToPointDistanceSquared(thData.Record.Features,dt->SecRec.Features,featuresCount);
			if (dist<maxDiffDist)
				maxDiffDist = dist;
		}

		// numar sa vad dintre cele similare cate sunt apropiate de el
		countClasified = 0;
		centroidRay = 0.0;
		for (gr=0;gr<sameClassCount;gr++)
		{
			idx = indexSameClass->Get(gr);
			if (con->GetRecord(dt->SecRec,idx)==false)
			{
				notif->Error("[%s] -> Unable to read record #%d",ObjectName,idx);
				return false;
			}
			dist = GML::ML::VectorOp::PointToPointDistanceSquared(thData.Record.Features,dt->SecRec.Features,featuresCount);
			if (dist<maxDiffDist)
			{
				countClasified++;
				if (dist>centroidRay)
					centroidRay = dist;
			}
		}

		// salvez datele
		cd[idxWork].corectelyClasify = countClasified;
		cd[idxWork].ClosestDifferentDistance = maxDiffDist;
		cd[idxWork].MostDistantSimilarDistance = centroidRay;
		// salvez cate ar putea clasifica
		if (thData.ThreadID==0)
			notif->SetProcent(tr,workCount);
	}
	if (thData.ThreadID==0)
		notif->EndProcent();
	return true;
}
bool Centroid::SaveResultsToDisk()
{
	GML::Utils::File		f;
	UInt32					tr,count;
	double					label;
	GML::Utils::GString		all,temp;

	notif->Info("[%s] -> Saving results to %s",ObjectName,ResultFileName.GetText());
	if (all.Create(0x4000)==false)
	{
		notif->Error("[%s] -> Unable to alloc 0x4000 bytes for caching ...",ObjectName);
		return false;
	}
	if (BuildHeaders(all)==false)
	{
		notif->Error("[%s] -> Unable to add string to cache",ObjectName);
		return false;
	}
	if (all.Add("\n")==false)
	{
		notif->Error("[%s] -> Unable to add string to cache",ObjectName);
		return false;
	}
	if (f.Create(ResultFileName.GetText())==false)
	{
		notif->Error("[%s] -> Unable to create: %s",ObjectName,ResultFileName.GetText());
		return false;
	}	
	count = 0;
	for (tr=0;tr<distInfo.Len();tr++)
	{
		if (con->GetRecordLabel(label,distInfo[tr].index)==false)
		{
			notif->Error("[%s] -> Unable to read label for record #%d",ObjectName,distInfo[tr].index);
			return false;
		}
		if (label==1)
		{
			if (distInfo[tr].corectelyClasify<minPositiveElements)
				continue;
		} else {
			if (distInfo[tr].corectelyClasify<minNegativeElements)
				continue;
		}
		if (distInfo[tr].corectelyClasify==0)
			continue;
		count++;
		if (BuildLineRecord(&distInfo[tr],temp)==false)
		{
			notif->Error("[%s] -> Unable to alloc create line #%d",ObjectName,tr);
			return false;
		}
		if (all.Add(&temp)==false)
		{
			notif->Error("[%s] -> Unable to add string to cache",ObjectName);
			return false;
		}
		if (all.Add("\n")==false)
		{
			notif->Error("[%s] -> Unable to add string to cache",ObjectName);
			return false;
		}
		if (all.Len()>0x3000)
		{
			if (f.Write(all.GetText(),all.Len())==false)
			{
				notif->Error("[%s] -> Unable to write data to %s",ObjectName,ResultFileName.GetText());
				f.Close();
				DeleteFile(ResultFileName.GetText());
				return false;
			}
			all.Set("");
		}
	}
	if (all.Len()>0)
	{
		if (f.Write(all.GetText(),all.Len())==false)
		{
			notif->Error("[%s] -> Unable to write data to %s",ObjectName,ResultFileName.GetText());
			f.Close();
			DeleteFile(ResultFileName.GetText());
			return false;
		}
	}
	notif->Info("[%s] -> %d records written to %s",ObjectName,count,ResultFileName.GetText());
	return true;
}
bool Centroid::SaveCentroid(CentroidDistances *cd,char *fileName)
{
	GML::Utils::AttributeList	attr;
	GML::Utils::GString			str;

	if (con->GetRecord(MainRecord,cd->index,GML::ML::RECORD_STORE_HASH)==false)
	{
		notif->Error("[%s] -> Unable to read record #%d",ObjectName,cd->index);
		return false;
	}
	if (MainRecord.Hash.ToString(str)==false)
	{
		notif->Error("[%s] -> Unable to convert hash to string",ObjectName);
		return false;
	}

	if (attr.AddDouble("MinRay",cd->MostDistantSimilarDistance)==false)
	{
		notif->Error("[%s] -> Unable to add 'MinRay' attribute to %s",ObjectName,fileName);
		return false;
	}
	if (attr.AddDouble("MaxRay",cd->ClosestDifferentDistance)==false)
	{
		notif->Error("[%s] -> Unable to add 'MaxRay' attribute to %s",ObjectName,fileName);
		return false;
	}
	if (attr.AddUInt32("Elements",cd->corectelyClasify)==false)
	{
		notif->Error("[%s] -> Unable to add 'MaxRay' attribute to %s",ObjectName,fileName);
		return false;
	}
	if (attr.AddString("Hash",str.GetText())==false)
	{
		notif->Error("[%s] -> Unable to add 'Hash' attribute to %s",ObjectName,fileName);
		return false;
	}
	if (attr.AddDouble("ClassType",MainRecord.Label)==false)
	{
		notif->Error("[%s] -> Unable to add 'ClassType' attribute to %s",ObjectName,fileName);
		return false;
	}
	if (attr.AddAttribute("Center",MainRecord.Features,GML::Utils::AttributeList::DOUBLE,MainRecord.FeatCount)==false)
	{
		notif->Error("[%s] -> Unable to add 'Center' attribute to %s",ObjectName,fileName);
		return false;
	}
	if (attr.Save(fileName)==false)
	{
		notif->Error("[%s] -> Unable to create %s",ObjectName,fileName);
		return false;
	}
	return true;
}
bool Centroid::SaveCentroids()
{
	UInt32					tr,count;
	GML::Utils::GString		str;
	double					label;
	char					cType;

	notif->Info("[%s] -> Saving centroids to %s",ObjectName,CentroidFileName.GetText());
	count = 0;
	for (tr=0;tr<distInfo.Len();tr++)
	{
		if (con->GetRecordLabel(label,distInfo[tr].index)==false)
		{
			notif->Error("[%s] -> Unable to read label for record #%d",ObjectName,distInfo[tr].index);
			return false;
		}
		if (label==1)
		{
			if (distInfo[tr].corectelyClasify<minPositiveElements)
				continue;
			cType = 'P';
		} else {
			if (distInfo[tr].corectelyClasify<minNegativeElements)
				continue;
			cType = 'N';
		}
		if (distInfo[tr].corectelyClasify==0)
			continue;
		if (str.SetFormated("%s_%06d_%c_%06d.centroid",CentroidFileName.GetText(),distInfo[tr].corectelyClasify,cType,distInfo[tr].index)==false)
		{
			notif->Error("[%s] -> Unable to create centroid name",ObjectName);
			return false;
		}
		if (SaveCentroid(&distInfo[tr],str.GetText())==false)
			return false;
		count++;
	}
	notif->Info("[%s] -> %d centroids created",ObjectName,count);
	return true;
}
void Centroid::Compute()
{
	switch (ClassType)
	{
		case CLASSTYPE_POSITIVE:
			notif->Info("[%s] -> Analizing positive records ...",ObjectName);
			ExecuteParalelCommand(FIND_CENTROID_POZ);
			break;
		case CLASSTYPE_NEGATIVE:
			notif->Info("[%s] -> Analizing negative records ...",ObjectName);
			ExecuteParalelCommand(FIND_CENTROID_NEG);
			break;
		case CLASSTYPE_BOTH:
			notif->Info("[%s] -> Analizing positive records ...",ObjectName);
			ExecuteParalelCommand(FIND_CENTROID_POZ);
			notif->Info("[%s] -> Analizing negative records ...",ObjectName);
			ExecuteParalelCommand(FIND_CENTROID_NEG);
			break;
	}
	
	if (SortResults)
	{
		notif->Info("[%s] -> Sorting ...",ObjectName);
		distInfo.Sort(CentroidDistanceCompareFunction,false);		
	}
	if (SaveResults!=SAVE_RESULTS_NONE)
		SaveResultsToDisk();
	SaveCentroids();
}

void Centroid::OnExecute()
{
	if (Command==1)	//Compute
	{
		Compute();
		return;
	}
	notif->Error("[%s] -> Unknown command ID: %d",ObjectName,Command);
}