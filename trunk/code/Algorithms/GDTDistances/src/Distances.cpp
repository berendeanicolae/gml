#include "Distances.h"

void removeAllNodPoints(NodPoint **prim)
{
	if (*prim == NULL)
		return;

	if ((*prim)->urm != NULL)
		removeAllNodPoints(&((*prim)->urm));	
	delete (*prim);
	*prim = NULL;	
}

bool addNewNodPoint(NodPoint **prim, int value)
{
	if(*prim == NULL)  
	{		
		*prim = new NodPoint;
		if(*prim == NULL)
			return false;  
		(*prim)->index = value;
		(*prim)->urm = NULL;
		return true;
	}
	
	NodPoint *it = *prim;
	while(it->urm != NULL)
		it = it->urm;
	it->urm = new NodPoint;
	it = it->urm;
	if(it == NULL)
		return false;
	it->index = value;
	it->urm = NULL;	
	return true;
}

//===============================================================================
Distances::Distances()
{
	ObjectName = "Distances";
	featWeight = NULL;

	LinkPropertyToUInt32("Method",Method,0,"!!LIST:PositiveToNegativeDistance=0,DistanceTablePositiveToNegative,DistanceTablePositiveToPositive,DistanceTableNegativeToNegative,DistanceTableNegativeToPositive,DistanceToPlan,ClosestMalwareClean!!");
	LinkPropertyToUInt32("ClosestPointArrayLen", ClosestPointArrayLen, 2, "Lenght for the static array from ClosestPoints structure");
	LinkPropertyToDouble("MinDistance",MinDist,0,"Minimal distance");
	LinkPropertyToDouble("MaxDistance",MaxDist,1,"Maximal distance");
	
	LinkPropertyToString("DistanceTableFileName",DistanceTableFileName,"","Name of the file names where the distances will be written");
	LinkPropertyToBool  ("MergeDistanceTableFiles",MergeDistanceTableFiles,true,"If set all of the distance table files will be merge into one");
	LinkPropertyToBool  ("UseWeightsForFeatures",UseWeightsForFeatures,false,"Specifyes if weights for features should be used.");
	
	LinkPropertyToString("FeaturesWeightFile",FeaturesWeightFile,"","Name of the file that contains the weights for features!");
	LinkPropertyToString("PlanFile",PlanFile,"","Name of the file that contains the informations about the plan");

	AddDistanceProperties();

}
double Distances::GetDistance(GML::ML::MLRecord &r1,GML::ML::MLRecord &r2)
{
	if (UseWeightsForFeatures)
		return GML::Algorithm::IMLAlgorithm::GetDistance(r1,r2,featWeight);
	else
		return GML::Algorithm::IMLAlgorithm::GetDistance(r1,r2);
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

	if(!(con->CreateMlRecord(MainRecord)))
	{
		notif->Error("[%s] -> Unable to create MainRecord",ObjectName);
		return false;
	}
	FeatCount = con->GetFeatureCount();

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
		dist = GML::ML::VectorOp::PointToPlaneDistanceSquaredSigned(plan.Weight,thData.Record.Features,plan.Count,plan.Bias);
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
		if (con->GetRecord(thData.Record,i1Poz,GML::ML::ConnectorFlags::STORE_HASH)==false)
		{
			notif->Error("[%s] -> Unable to read record #%d",ObjectName,i1Poz);
			return false;
		}
		for (gr=0;gr<i2Count;gr++)
		{
			i2Poz = i2.Get(gr);
			if (con->GetRecord(dt->SetRec,i2Poz,GML::ML::ConnectorFlags::STORE_HASH)==false)
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
bool Distances::ComputeClosestPositiveNegative(GML::Algorithm::MLThreadData &thData)
{	
	UInt32				positiveCount, negativeCount, i, j, idxPoz, idxNeg;
	DistThreadData*		dt = (DistThreadData *)thData.Context;
	double				dist;

	positiveCount = indexesPozitive.Len();
	//negativeCount = indexesNegative.Len();
	if (thData.ThreadID==0)
		notif->StartProcent("[%s] -> Computing ... ",ObjectName);
	
	for (i=thData.ThreadID; i<positiveCount; i+=threadsCount)
	{						
		//negativePoints[i].dist = con->GetFeatureCount();
		//negativePoints[i].count = 0;
		negativeCount = indexesNegative.Len();
		idxPoz = indexesPozitive.Get(i);
		if (con->GetRecord(thData.Record,idxPoz)==false)
		{
			notif->Error("[%s] -> Unable to read record #%d",ObjectName,idxPoz);
			return false;
		}
		//for (j=0; j<negativeCount; j++)
		for (j=0; negativeCount--; j++)
		{
			idxNeg = indexesNegative.Get(j);
			if (con->GetRecord(dt->SetRec,idxNeg)==false)
			{
				notif->Error("[%s] -> Unable to read record #%d",ObjectName,idxNeg);
				return false;
			}
			dist = GetDistance(thData.Record,dt->SetRec);
			if (dist < negativePoints[i].dist)		
			{
				negativePoints[i].dist = dist;
				negativePoints[i].firstPoints[0] = j;
				//negativePoints[i].firstPoints[1] = -1;				
				if (negativePoints[i].count > ClosestPointArrayLen)								
					removeAllNodPoints(&(negativePoints[i].prim));					
				negativePoints[i].count = 1;
			//}else if (dist == negativePoints[i].dist)
			}else if (!(dist - negativePoints[i].dist))
			{
				if (negativePoints[i].count < ClosestPointArrayLen)				
					negativePoints[i].firstPoints[negativePoints[i].count++] = j;
				else
				{								
					if (!addNewNodPoint(&(negativePoints[i].prim), j))
					{
						notif->Error("[%s] -> Unable to append to closest points for record:#%d",ObjectName,i);
						return false;
					}
					negativePoints[i].count++;
				}
			}  
		}
		if (thData.ThreadID==0)
			notif->SetProcent(i,positiveCount);		
	}
	if (thData.ThreadID==0)
		notif->EndProcent();

	return true;
}

bool Distances::ExportNewPair(GML::Utils::File *f, char *fileName, UInt32 cleanId, UInt32 malID, double dist)
{
	GML::Utils::GString		temp, hashTmp;

	if (con->GetRecord(MainRecord,indexesNegative.Get(cleanId),GML::ML::ConnectorFlags::STORE_HASH)==false)
	{
		notif->Error("[%s] -> Unable to read record #%d",ObjectName,indexesNegative.Get(cleanId));
		return false;
	}
	if (MainRecord.Hash.ToString(hashTmp)==false)
	{
		notif->Error("[%s] -> Unable to convert hash to string",ObjectName);
		return false;
	}			
	if (temp.SetFormated("NP|%s|",hashTmp.GetText())==false)
	{
		notif->Error("[%s] -> Unable to add data to string",ObjectName);
		return false;
	}
	if (con->GetRecord(MainRecord,indexesPozitive.Get(malID),GML::ML::ConnectorFlags::STORE_HASH)==false)
	{
		notif->Error("[%s] -> Unable to read record #%d",ObjectName,indexesPozitive.Get(malID));
		return false;
	}
	if (MainRecord.Hash.ToString(hashTmp)==false)
	{
		notif->Error("[%s] -> Unable to convert hash to string",ObjectName);
		return false;
	}
	if (temp.AddFormated("%s|%lf\n",hashTmp.GetText(),dist)==false)
	{
		notif->Error("[%s] -> Unable to add data to string",ObjectName);
		return false;
	}	
	if ((*f).Write(temp.GetText(), temp.Len()) == false)
	{
		notif->Error("[%s] -> Unable to write to file %s",ObjectName, fileName);
		return false;
	}

	return true;
}
bool Distances::UpdateNegativePositive(UINT32 malId, UINT32 cleanId, double dist)
{
	if (dist < positivePoints[cleanId].dist)
	{
		positivePoints[cleanId].dist = dist;
		positivePoints[cleanId].firstPoints[0] = malId;
		//positivePoints[cleanId].firstPoints[1] = -1;
		if (positivePoints[cleanId].count > ClosestPointArrayLen)								
			removeAllNodPoints(&(positivePoints[cleanId].prim));									
		positivePoints[cleanId].count = 1;
		return true;
	//} else if (dist == positivePoints[cleanId].dist)
	} else if (!(dist - positivePoints[cleanId].dist))
	{
		if (positivePoints[cleanId].count < ClosestPointArrayLen)
		{
			positivePoints[cleanId].firstPoints[positivePoints[cleanId].count++] = malId;
			return true;
		} else
		{
			if (!addNewNodPoint(&(positivePoints[cleanId].prim), malId))
			{
				notif->Error("[%s] -> Unable to append to closest points for record:#%d",ObjectName,cleanId);
				return false;
			}
			positivePoints[cleanId].count++;
			return true;
		}
	}
	return true;	
}
bool Distances::MergeDistancesClosestPositiveNegative()
{
	UInt32	i, j, positiveCount;

	notif->Info("[%s] -> Merging distances...",ObjectName);

	positiveCount = indexesPozitive.Len();
	//for (i=0; i<positiveCount; i++)
	for (i=0; positiveCount--; i++)
	{
		//for (j=0; j<2; j++)
		//	if (negativePoints[i].firstPoints[j] != -1)
		for (j=0; negativePoints[i].count-- && j<ClosestPointArrayLen; j++)
			if (!UpdateNegativePositive(i, negativePoints[i].firstPoints[j], negativePoints[i].dist))
			{
				notif->Error("[%s] -> Unable to append to set closest malware for clean:#%d",ObjectName,indexesNegative.Get(negativePoints[i].firstPoints[j]));
				return false;
			}
		
		//if(!negativePoints[i].count)
		//	continue;

		NodPoint *p = negativePoints[i].prim;
		while(p!=NULL)  
		//while(negativePoints[i].count--)
		{
			if (!UpdateNegativePositive(i, p->index, negativePoints[i].dist))
				{
					notif->Error("[%s] -> Unable to append to set closest malware for clean:#%d",ObjectName,indexesNegative.Get(p->index));
					return false;
				}
			p = p->urm;
		}
	}

	return true;
}
bool Distances::SaveNegativePositive(char *fileName)
{
	GML::Utils::File		f;
	UInt32					i, j, negativeCount;
	UInt8					*ptr = RecordsStatus.GetVector();
	
	notif->Info("[%s] -> Start saving data to file: %s",ObjectName, fileName);

	if (f.Create(fileName) == false)		
		return false;

	negativeCount = indexesNegative.Len();	
	for (i=0; i<negativeCount; i++)
	{
		if(positivePoints[i].count == -1)
			continue;
		//for (j=0; j<ClosestPointArrayLen && j<positivePoints[i].count; j++)
		for (j=0; j<ClosestPointArrayLen && positivePoints[i].count--; j++)
		{
			if (ExportNewPair(&f, fileName, i, positivePoints[i].firstPoints[j], positivePoints[i].dist) == false)
			{				
				notif->Error("[%s] -> Unable to write new pair to file %s",ObjectName, fileName);
				return false;
			}
			ptr[indexesNegative.Get(i)] = 1;
			ptr[indexesPozitive.Get(positivePoints[i].firstPoints[j])] = 1;
		}

		//if(!positivePoints[i].count)
		//	continue;

		NodPoint *p = positivePoints[i].prim;
		while(p!=NULL)
		{
			if (ExportNewPair(&f, fileName, i, p->index, positivePoints[i].dist) == false)
			{
				notif->Error("[%s] -> Unable to write new pair to file %s",ObjectName, fileName);
				return false;
			}
			ptr[indexesNegative.Get(i)] = 1;
			ptr[indexesPozitive.Get(positivePoints[i].firstPoints[j])] = 1;
			p = p->urm;
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
		case METHOD_ClosestNegativePositive:
			ComputeClosestPositiveNegative(thData);
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
bool Distances::InitClosestPointsArray(ClosestPoints **toInit, int len)
{
	*toInit = new ClosestPoints[len];
	for (int i=0; len--; i++)
	{
		(*toInit)[i].prim = NULL;
		(*toInit)[i].count = 0;
		(*toInit)[i].firstPoints = new int[ClosestPointArrayLen];
		(*toInit)[i].dist = FeatCount;
	}

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
		case METHOD_ClosestNegativePositive:
			/*negativePoints = new ClosestPoints[indexesPozitive.Len()];
			memset(negativePoints, -1, indexesPozitive.Len()*sizeof(ClosestPoints));
			for (int i=0; i<indexesPozitive.Len(); i++)
				negativePoints[i].prim = NULL;
			*/
			InitClosestPointsArray(&negativePoints, indexesPozitive.Len());
			ExecuteParalelCommand(Method);
			InitClosestPointsArray(&positivePoints, indexesNegative.Len());
			/*
			positivePoints = new ClosestPoints[indexesNegative.Len()];
			memset(positivePoints, -1, indexesNegative.Len()*sizeof(ClosestPoints));
			for (int i=0; i<indexesNegative.Len(); i++)
			{ 
				positivePoints[i].prim = NULL;
				positivePoints[i].dist = FeatCount;
			}
			*/			
			MergeDistancesClosestPositiveNegative();			
			SaveNegativePositive(DistanceTableFileName.GetText());
			SaveHashResult(HashFileName.GetText(),HashFileType,RecordsStatus);
	}
	return false;
}