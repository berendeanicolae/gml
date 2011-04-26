#include "Distances.h"

Distances::Distances()
{
	ObjectName = "Distances";

	LinkPropertyToUInt32("Method",Method,0,"!!LIST:PositiveToNegativeDistance=0,DistanceTablePositiveToNegative,DistanceTablePositiveToPositive,DistanceTableNegativeToNegative,DistanceTableNegativeToPositive!!");
	LinkPropertyToDouble("MinDistance",MinDist,0,"Minimal distance");
	LinkPropertyToDouble("MaxDistance",MaxDist,1,"Maximal distance");
	LinkPropertyToString("DistanceTableFileName",DistanceTableFileName,"","Name of the file names where the distance will be written");

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
			dist = GML::ML::VectorOp::PointToPointDistanceSquared(thData.Record.Features,dt->SetRec.Features,featuresCount);
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
bool Distances::ComputeDistanceTable(GML::Algorithm::MLThreadData &thData,GML::Utils::Indexes &i1,GML::Utils::Indexes &i2)
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
		notif->Error("[%s] -> Unable to create file : %s ",ObjectName,thData.ThreadID,buf.GetText());
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
			dist = GML::ML::VectorOp::PointToPointDistanceSquared(thData.Record.Features,dt->SetRec.Features,featuresCount);
			if ((dist>=MinDist) && (dist<=MaxDist))
			{				
				if (thData.Record.Hash.ToString(tmp)==false)
				{
					notif->Error("[%s] -> Unable to convert hash to string",ObjectName);
					return false;
				}
				if (buf.AddFormated("%s|",tmp.GetText())==false)
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
void Distances::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand)
{
	switch (Method)
	{
		case METHOD_PositiveToNegativeDistance:
			ComputePositiveToNegativeDistance(thData);
			break;
		case METHOD_DistanceTablePositiveToNegative:
			ComputeDistanceTable(thData,indexesPozitive,indexesNegative);
			break;
		case METHOD_DistanceTablePositiveToPositive:
			ComputeDistanceTable(thData,indexesPozitive,indexesPozitive);
			break;
		case METHOD_DistanceTableNegativeToNegative:
			ComputeDistanceTable(thData,indexesNegative,indexesNegative);
			break;
		case METHOD_DistanceTableNegativeToPositive:
			ComputeDistanceTable(thData,indexesNegative,indexesPozitive);
			break;
	}
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
			return true;
		case METHOD_DistanceTablePositiveToPositive:
			if (DistanceTableFileName.Len()==0)
			{
				notif->Error("[%s] -> You need to set up the 'DistanceTableFileName' parameters",ObjectName);
				return false;
			}
			notif->Info("[%s] -> Computing table distances between positive and positive elements ...",ObjectName);
			ExecuteParalelCommand(Method);
			return true;
		case METHOD_DistanceTableNegativeToPositive:
			if (DistanceTableFileName.Len()==0)
			{
				notif->Error("[%s] -> You need to set up the 'DistanceTableFileName' parameters",ObjectName);
				return false;
			}
			notif->Info("[%s] -> Computing table distances between negative and positive elements ...",ObjectName);
			ExecuteParalelCommand(Method);
			return true;
		case METHOD_DistanceTableNegativeToNegative:
			if (DistanceTableFileName.Len()==0)
			{
				notif->Error("[%s] -> You need to set up the 'DistanceTableFileName' parameters",ObjectName);
				return false;
			}
			notif->Info("[%s] -> Computing table distances between negative and negative elements ...",ObjectName);
			ExecuteParalelCommand(Method);
			return true;
	}
	return false;
}
