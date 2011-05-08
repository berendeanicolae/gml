#include "Distances.h"

double	ProcDistance(double *p1,double *p2,UInt32 elements)
{
	double sum=0.0;;
	double total=0.0;
	while (elements>0)
	{
		if ((*p1)!=(*p2))
			sum+=1;
		if (((*p1)!=0.0) || ((*p2)!=0.0))
			total+=1;
		p1++;
		p2++;
		elements--;
	}
	return (double)(sum/total);
}
double	ProcDistance(double *p1,double *p2,UInt32 elements,double *pWeight)
{
	double sum=0.0;;
	double total=0.0;
	while (elements>0)
	{
		if ((*p1)!=(*p2))
			sum+=(*pWeight);
		if (((*p1)!=0.0) || ((*p2)!=0.0))
			total+=(*pWeight);
		p1++;
		p2++;
		pWeight++;
		elements--;
	}
	return (double)(sum/total);
}
//===============================================================================
Distances::Distances()
{
	ObjectName = "Distances";
	featWeight = NULL;

	LinkPropertyToUInt32("Method",Method,0,"!!LIST:PositiveToNegativeDistance=0,DistanceTablePositiveToNegative,DistanceTablePositiveToPositive,DistanceTableNegativeToNegative,DistanceTableNegativeToPositive,DistanceToPlan!!");
	LinkPropertyToDouble("MinDistance",MinDist,0,"Minimal distance");
	LinkPropertyToDouble("MaxDistance",MaxDist,1,"Maximal distance");
	LinkPropertyToDouble("Power",Power,1,"Power parameter in Minkowski distance");
	LinkPropertyToString("DistanceTableFileName",DistanceTableFileName,"","Name of the file names where the distance will be written");
	LinkPropertyToBool  ("MergeDistanceTableFiles",MergeDistanceTableFiles,true,"If set all of the distance table files will be merge into one");
	LinkPropertyToBool  ("UseWeightsForFeatures",UseWeightsForFeatures,false,"Specifyes if weights for features should be used.");
	LinkPropertyToUInt32("DistanceFunction",DistanceFunction,DIST_FUNC_Euclidean,"!!LIST:Manhattan=0,Euclidean,EuclideanSquared,Minkowski,ProcDifference!!");
	LinkPropertyToString("FeaturesWeightFile",FeaturesWeightFile,"","Name of the file that contains the weights for features!");
	LinkPropertyToString("PlanFile",PlanFile,"","Name of the file that contains the informations about the plan");

}
double Distances::GetDistance(GML::ML::MLRecord &r1,GML::ML::MLRecord &r2)
{
	if (UseWeightsForFeatures)
	{
		switch (DistanceFunction)
		{
			case DIST_FUNC_Manhattan:
				return GML::ML::VectorOp::ManhattanDistance(r1.Features,r2.Features,r1.FeatCount,featWeight);
			case DIST_FUNC_Euclidean:
				return GML::ML::VectorOp::EuclideanDistance(r1.Features,r2.Features,r1.FeatCount,featWeight);
			case DIST_FUNC_Euclidean_Square:
				return pow(GML::ML::VectorOp::EuclideanDistance(r1.Features,r2.Features,r1.FeatCount,featWeight),2);
			case DIST_FUNC_Minkowski:
				return GML::ML::VectorOp::MinkowskiDistance(r1.Features,r2.Features,r1.FeatCount,Power,featWeight);
			case DIST_FUNC_ProcDifference:
				return ProcDistance(r1.Features,r2.Features,r1.FeatCount,featWeight);
		}
	} else {
		switch (DistanceFunction)
		{
			case DIST_FUNC_Manhattan:
				return GML::ML::VectorOp::ManhattanDistance(r1.Features,r2.Features,r1.FeatCount);
			case DIST_FUNC_Euclidean:
				return GML::ML::VectorOp::EuclideanDistance(r1.Features,r2.Features,r1.FeatCount);
			case DIST_FUNC_Euclidean_Square:
				return GML::ML::VectorOp::EuclideanDistanceSquared(r1.Features,r2.Features,r1.FeatCount);
			case DIST_FUNC_Minkowski:
				return GML::ML::VectorOp::MinkowskiDistance(r1.Features,r2.Features,r1.FeatCount,Power);
			case DIST_FUNC_ProcDifference:
				return ProcDistance(r1.Features,r2.Features,r1.FeatCount);
		}
	}
	return 0;
}
bool Distances::LoadPlan(char *fileName,Plan &pv)
{
	GML::Utils::AttributeList	tempAttr;

	if ((pv.Weight = new double[con->GetFeatureCount()])==NULL)
	{
		notif->Error("[%s] -> Unable to alloc (%d) bytes for weights : %s",ObjectName,con->GetFeatureCount()*sizeof(double));
		return false;
	}
	pv.Count = con->GetFeatureCount();
	if (tempAttr.Load(fileName)==false)
	{
		notif->Error("[%s] -> Unable to load : %s",ObjectName,fileName);
		return false;
	}
	if (tempAttr.UpdateDouble("Bias",pv.Bias,true)==false)
	{
		notif->Error("[%s] -> Unable to update 'bias' value from %s",ObjectName,fileName);
		return false;
	}
	if (tempAttr.Update("Weight",pv.Weight,sizeof(double)*pv.Count)==false)
	{
		notif->Error("[%s] -> Unable to update 'Weight' value from %s",ObjectName,fileName);
		return false;
	}
	notif->Info("[%s] -> %s loaded ok",ObjectName,fileName);
	return true;
}
bool Distances::LoadFeatureWeightFile()
{
	GML::Utils::AttributeList	attrList;
	UInt32						tr;

	if ((featWeight = new double[con->GetFeatureCount()])==NULL)
	{
		notif->Error("[%s] -> Unable to alloc featWeight[%d]",ObjectName,con->GetFeatureCount());
		return false;
	}
	if (attrList.Load(FeaturesWeightFile.GetText())==false)
	{
		notif->Error("[%s] -> Unable to load FeaturesWeightFile : %s",ObjectName,FeaturesWeightFile.GetText());
		return false;
	}
	if (attrList.Update("Weight",featWeight,sizeof(double)*con->GetFeatureCount())==false)
	{
		notif->Error("[%s] -> Unable to update 'Weight' property from %s",ObjectName,FeaturesWeightFile.GetText());
		return false;
	}

	notif->Info("[%s] -> %s loaded ok ",ObjectName,FeaturesWeightFile.GetText());
	// facem si un mic test

	for (tr=0;tr<con->GetFeatureCount();tr++)
	{
		if (featWeight[tr]<=0.0)
		{
			notif->Error("[%s] -> Feature Weight %d is lower than 0.0 (%.4lf)",ObjectName,tr,featWeight[tr]);
		}	
	}
	return true;
}
bool Distances::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	DistThreadData *d = new DistThreadData();
	if (d==NULL)
		return false;
	if (con->CreateMlRecord(d->SetRec)==false)
		return false;
	thData.Context = d;
	return true;
}
bool Distances::OnInit()
{
	if (CreatePozitiveAndNegativeIndexes()==false)
		return false;
	if (UseWeightsForFeatures)
	{
		if (LoadFeatureWeightFile()==false)
			return false;
	}
	if (Method==METHOD_DistanceToPlan)
	{
		if (planDist.Create(con->GetRecordCount())==false)
		{
			notif->Error("[%s] -> Unable to allocate %d distances ",ObjectName,con->GetRecordCount());
			return false;
		}
		if (planDist.Resize(con->GetRecordCount())==false)
		{
			notif->Error("[%s] -> Unable to allocate %d distances (2)",ObjectName,con->GetRecordCount());
			return false;
		}
		memset(planDist.GetVector(),0,sizeof(double) * con->GetRecordCount());
		if (LoadPlan(PlanFile.GetText(),plan)==false)
			return false;
	}
	return true;
}
bool Distances::ComputePositiveToNegativeDistance(GML::Algorithm::MLThreadData &thData)
{
	UInt32				tr,gr,featuresCount,pozitiveCount,negativeCount,idxPoz,idxNeg;
	DistThreadData*		dt = (DistThreadData *)thData.Context;
	double				dist;
	UInt8				*ptr = RecordsStatus.GetVector();

	featuresCount = con->GetFeatureCount();
	pozitiveCount = indexesPozitive.Len();
	negativeCount = indexesNegative.Len();
	if (thData.ThreadID==0)
		notif->StartProcent("[%s] -> Computing ... ",ObjectName);
	for (tr=thData.ThreadID;tr<pozitiveCount;tr+=threadsCount)
	{
		idxPoz = indexesPozitive.Get(tr);
		if (con->GetRecord(thData.Record,idxPoz)==false)
		{
			notif->Error("[%s] -> Unable to read record #%d",ObjectName,idxPoz);
			return false;
		}
		for (gr=0;gr<negativeCount;gr++)
		{
			idxNeg = indexesNegative.Get(gr);
			if (con->GetRecord(dt->SetRec,idxNeg)==false)
			{
				notif->Error("[%s] -> Unable to read record #%d",ObjectName,idxNeg);
				return false;
			}
			dist = GetDistance(thData.Record,dt->SetRec);
			if ((dist>=MinDist) && (dist<=MaxDist))
			{				
				ptr[idxPoz] = 1;
				ptr[idxNeg] = 1;
			}
		}
		if (thData.ThreadID==0)
			notif->SetProcent(tr,pozitiveCount);
	}
	if (thData.ThreadID==0)
		notif->EndProcent();
	return true;
}
bool Distances::ComputeDistanceToPlan(GML::Algorithm::MLThreadData &thData)
{
	UInt32				tr,sz;
	DistThreadData*		dt = (DistThreadData *)thData.Context;
	double				dist;
	double				*dPlan = planDist.GetVector();
	UInt8				*ptr = RecordsStatus.GetVector();

	sz = thData.Range.Size();
	if (thData.ThreadID==0)
		notif->StartProcent("[%s] -> Computing ... ",ObjectName);

	for (tr=thData.Range.Start;tr<thData.Range.End;tr++)
	{
		if (con->GetRecord(thData.Record,tr)==false)
		{
			notif->Error("[%s] -> Unable to read record #%d",ObjectName,tr);
			return false;
		}
		dist = GML::ML::VectorOp::PointToPlaneDistanceSigned(plan.Weight,thData.Record.Features,plan.Count,plan.Bias);
		if ((dist>=MinDist) && (dist<=MaxDist))
		{
			dPlan[tr] = dist;
			ptr[tr] = 1;
		} else {
			dPlan[tr] = 0; // sau NaN
			ptr[tr]=0;
		}


		if (thData.ThreadID==0)
			notif->SetProcent(tr-thData.Range.Start,sz);
	}
	if (thData.ThreadID==0)
		notif->EndProcent();
	return true;
}
bool Distances::ComputeDistanceTable(GML::Algorithm::MLThreadData &thData,GML::Utils::Indexes &i1,GML::Utils::Indexes &i2,char *Type)
{
	UInt32				tr,gr,featuresCount,i1Count,i2Count,i1Poz,i2Poz;
	DistThreadData*		dt = (DistThreadData *)thData.Context;
	double				dist;
	UInt8				*ptr = RecordsStatus.GetVector();
	GML::Utils::File	f;
	GML::Utils::GString	buf,tmp;

	if (buf.Create(32000)==false)
	{
		notif->Error("[%s] -> Thread %d was unable to alloc data for internal bufferes ",ObjectName,thData.ThreadID);
		return false;
	}
	if (buf.SetFormated("%s.%d",DistanceTableFileName.GetText(),thData.ThreadID)==false)
	{
		notif->Error("[%s] -> Thread %d was unable to create table file : %s ",ObjectName,thData.ThreadID,DistanceTableFileName.GetText());
		return false;
	}
	if (f.Create(buf.GetText(),true)==false)
	{
		notif->Error("[%s] -> Unable to create file : %s ",ObjectName,buf.GetText());
		return false;
	}

	
	buf.Set("");
	featuresCount = con->GetFeatureCount();
	i1Count = i1.Len();
	i2Count = i2.Len();

	if (thData.ThreadID==0)
		notif->StartProcent("[%s] -> Computing ... ",ObjectName);
	for (tr=thData.ThreadID;tr<i1Count;tr+=threadsCount)
	{
		i1Poz = i1.Get(tr);
		if (con->GetRecord(thData.Record,i1Poz,GML::ML::RECORD_STORE_HASH)==false)
		{
			notif->Error("[%s] -> Unable to read record #%d",ObjectName,i1Poz);
			return false;
		}
		for (gr=0;gr<i2Count;gr++)
		{
			i2Poz = i2.Get(gr);
			if (con->GetRecord(dt->SetRec,i2Poz,GML::ML::RECORD_STORE_HASH)==false)
			{
				notif->Error("[%s] -> Unable to read record #%d",ObjectName,i2Poz);
				return false;
			}
			dist = GetDistance(thData.Record,dt->SetRec);
			
			if ((dist>=MinDist) && (dist<=MaxDist))
			{		
				ptr[i1Poz] = 1;
				ptr[i2Poz] = 1;
				if (thData.Record.Hash.ToString(tmp)==false)
				{
					notif->Error("[%s] -> Unable to convert hash to string",ObjectName);
					return false;
				}
				if (buf.AddFormated("%s|%s|",Type,tmp.GetText())==false)
				{
					notif->Error("[%s] -> Unable to add data to string",ObjectName);
					return false;
				}
				if (dt->SetRec.Hash.ToString(tmp)==false)
				{
					notif->Error("[%s] -> Unable to convert hash to string",ObjectName);
					return false;
				}
				if (buf.AddFormated("%s|%lf\n",tmp.GetText(),dist)==false)
				{
					notif->Error("[%s] -> Unable to add data to string",ObjectName);
					return false;
				}
				// scriu datele
				if (buf.Len()>30000)
				{
					if (f.Write(buf.GetText(),buf.Len())==false)
					{
						f.Close();
						notif->Error("[%s] -> Unable to write data to table file",ObjectName);
						return false;
					}
					buf.Set("");
				}
			}
		}
		if (thData.ThreadID==0)
			notif->SetProcent(tr,i1Count);
	}
	if (thData.ThreadID==0)
		notif->EndProcent();
	// scriu datele
	if (buf.Len()>0)
	{
		if (f.Write(buf.GetText(),buf.Len())==false)
		{
			f.Close();
			notif->Error("[%s] -> Unable to write data to table file",ObjectName);
			return false;
		}
	}
	f.Close();
	return true;
}
bool Distances::MergeDistances()
{
	GML::Utils::GString		fName,rName;
	GML::Utils::File		f,fr;
	unsigned char			buf[0x10000];
	UInt32					read,tr;

	if (fName.SetFormated("%s",DistanceTableFileName.GetText())==false)
	{
		notif->Error("[%s] -> Unable to create table file : %s ",ObjectName,DistanceTableFileName.GetText());
		return false;
	}
	if (f.Create(fName.GetText(),true)==false)
	{
		notif->Error("[%s] -> Unable to create file : %s ",ObjectName,fName.GetText());
		return false;
	}
	for (tr=0;tr<threadsCount;tr++)
	{
		if (rName.SetFormated("%s.%d",DistanceTableFileName.GetText(),tr)==false)
		{
			notif->Error("[%s] -> Unable to create table file : %s (%d)",ObjectName,DistanceTableFileName.GetText(),tr);
			f.Close();
			DeleteFile(fName.GetText());
			return false;
		}
		if (fr.OpenRead(rName.GetText(),true)==false)
		{
			notif->Error("[%s] -> Unable to create file : %s ",ObjectName,rName.GetText());
			f.Close();
			DeleteFile(fName.GetText());
			return false;
		}
		notif->Info("[%s] -> Adding : %s",ObjectName,rName.GetText());
		do
		{
			if (fr.Read(buf,0x10000,&read)==false)
			{
				notif->Error("[%s] -> Unable to read from file : %s ",ObjectName,rName.GetText());
				f.Close();
				DeleteFile(fName.GetText());
				return false;
			}
			if (read>0)
			{
				if (f.Write(buf,read)==false)
				{
					notif->Error("[%s] -> Unable to write to file : %s ",ObjectName,fName.GetText());
					f.Close();
					DeleteFile(fName.GetText());
					return false;
				}
			}
		} while (read>0);
		fr.Close();
		DeleteFileA(rName.GetText());
	}
	f.Close();
	notif->Info("[%s] -> %s created ok.",ObjectName,fName.GetText());
	return true;
}
void Distances::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand)
{
	switch (Method)
	{
		case METHOD_PositiveToNegativeDistance:
			ComputePositiveToNegativeDistance(thData);
			break;
		case METHOD_DistanceTablePositiveToNegative:
			ComputeDistanceTable(thData,indexesPozitive,indexesNegative,"PN");
			break;
		case METHOD_DistanceTablePositiveToPositive:
			ComputeDistanceTable(thData,indexesPozitive,indexesPozitive,"PP");
			break;
		case METHOD_DistanceTableNegativeToNegative:
			ComputeDistanceTable(thData,indexesNegative,indexesNegative,"NN");
			break;
		case METHOD_DistanceTableNegativeToPositive:
			ComputeDistanceTable(thData,indexesNegative,indexesPozitive,"NP");
			break;
		case METHOD_DistanceToPlan:
			ComputeDistanceToPlan(thData);
			break;
	}
}
bool Distances::SavePlanDistances()
{
	GML::Utils::File		f;
	GML::Utils::GString		fName,temp,hash;
	UInt32					tr,count = 0;
	GML::DB::RecordHash		rHash;
	double					Label;


	if ((fName.SetFormated("%s_plan.dist",DistanceTableFileName.GetText())==false) || (temp.Create(0x10000)==false))
	{
		notif->Error("[%s] -> Unable to alloc memory for plane distance file name",ObjectName);
		return false;
	}
	if (f.Create(fName.GetText())==false)
	{
		notif->Error("[%s] -> Unable to create: %s",ObjectName,fName.GetText());
		return false;
	}
	for (tr=0;tr<con->GetRecordCount();tr++)
	{
		if (RecordsStatus[tr]==0)
			continue;
		if (con->GetRecordHash(rHash,tr)==false)
		{
			notif->Error("[%s] -> Unable to read record hash for #%d",ObjectName,tr);
			return false;
		}
		if (con->GetRecordLabel(Label,tr)==false)
		{
			notif->Error("[%s] -> Unable to read record hash for #%d",ObjectName,tr);
			return false;
		}
		if (rHash.ToString(hash)==false)
		{
			notif->Error("[%s] -> Unable to convert record hash for #%d",ObjectName,tr);
			return false;
		}
		if (temp.AddFormated("%s|%.3lf|%lf\n",hash.GetText(),Label,planDist[tr])==false)
		{
			notif->Error("[%s] -> Unable to convert record hash for #%d",ObjectName,tr);
			return false;
		}
		count++;
		if (temp.Len()>64000)
		{
			if (f.Write(temp,temp.Len())==false)
			{
				notif->Error("[%s] -> Unable to write to %s",ObjectName,fName.GetText());
				return false;
			}
			temp.Truncate(0);
			temp.Set("");
		}
	}
	if (f.Write(temp,temp.Len())==false)
	{
		notif->Error("[%s] -> Unable to write to %s",ObjectName,fName.GetText());
		return false;
	}
	f.Close();
	notif->Info("[%s] -> %d records written in %s ",ObjectName,count,fName.GetText());
	return true;
}
bool Distances::OnCompute()
{
	switch (Method)
	{
		case METHOD_PositiveToNegativeDistance:
			ExecuteParalelCommand(Method);
			SaveHashResult(HashFileName.GetText(),HashFileType,RecordsStatus);
			return true;
		case METHOD_DistanceTablePositiveToNegative:
			if (DistanceTableFileName.Len()==0)
			{
				notif->Error("[%s] -> You need to set up the 'DistanceTableFileName' parameters",ObjectName);
				return false;
			}
			notif->Info("[%s] -> Computing table distances between positive and negative elements ...",ObjectName);
			ExecuteParalelCommand(Method);
			if (MergeDistanceTableFiles)
				MergeDistances();
			SaveHashResult(HashFileName.GetText(),HashFileType,RecordsStatus);
			return true;
		case METHOD_DistanceTablePositiveToPositive:
			if (DistanceTableFileName.Len()==0)
			{
				notif->Error("[%s] -> You need to set up the 'DistanceTableFileName' parameters",ObjectName);
				return false;
			}
			notif->Info("[%s] -> Computing table distances between positive and positive elements ...",ObjectName);
			ExecuteParalelCommand(Method);
			if (MergeDistanceTableFiles)
				MergeDistances();
			SaveHashResult(HashFileName.GetText(),HashFileType,RecordsStatus);
			return true;
		case METHOD_DistanceTableNegativeToPositive:
			if (DistanceTableFileName.Len()==0)
			{
				notif->Error("[%s] -> You need to set up the 'DistanceTableFileName' parameters",ObjectName);
				return false;
			}
			notif->Info("[%s] -> Computing table distances between negative and positive elements ...",ObjectName);
			ExecuteParalelCommand(Method);
			if (MergeDistanceTableFiles)
				MergeDistances();
			SaveHashResult(HashFileName.GetText(),HashFileType,RecordsStatus);
			return true;
		case METHOD_DistanceTableNegativeToNegative:
			if (DistanceTableFileName.Len()==0)
			{
				notif->Error("[%s] -> You need to set up the 'DistanceTableFileName' parameters",ObjectName);
				return false;
			}
			notif->Info("[%s] -> Computing table distances between negative and negative elements ...",ObjectName);
			ExecuteParalelCommand(Method);
			if (MergeDistanceTableFiles)
				MergeDistances();
			SaveHashResult(HashFileName.GetText(),HashFileType,RecordsStatus);
			return true;
		case METHOD_DistanceToPlan:
			ExecuteParalelCommand(Method);
			SaveHashResult(HashFileName.GetText(),HashFileType,RecordsStatus);
			if (DistanceTableFileName.Len()!=0)
			{
				if (SavePlanDistances()==false)
					return false;
			}
			return true;
	}
	return false;
}