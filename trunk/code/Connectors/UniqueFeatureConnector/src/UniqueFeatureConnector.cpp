#include "UniqueFeatureConnector.h"

int FeatInfoCompare(FeatInfo &f1,FeatInfo &f2)
{
	return memcmp(f1.Hash.Hash.bValue,f2.Hash.Hash.bValue,16);
}
//==============================================================================
UniqueFeatureConnector::UniqueFeatureConnector()
{
	ObjectName = "UniqueFeatureConnector";
}
UniqueFeatureConnector::~UniqueFeatureConnector()
{
	Close();
}
bool UniqueFeatureConnector::AnalizeSubList(UInt32 start,UInt32 end)
{
	double	Label;
	UInt32	Pozitive = 0,Negative = 0;
	UInt32	curent = start;
	UInt32	firstNegative,firstPozitive;

	firstNegative = firstPozitive = 0xFFFFFFFF;
	while (curent<end)
	{
		if (conector->GetRecordLabel(Label,FList[curent].Index)==false)
		{
			notifier->Error("[%s] -> Unable to read label for record #%d!",ObjectName,FList[curent].Index);
			return false;
		}
		if (Label==1.0)
		{
			Pozitive++;
			if (firstPozitive == 0xFFFFFFFF)
				firstPozitive = FList[curent].Index;
		} else {
			Negative++;
			if (firstNegative == 0xFFFFFFFF)
				firstNegative = FList[curent].Index;
		}
		curent++;
	}
	// cazuri:
	if ((Pozitive>0) && (Negative==0))
		return Indexes.Push(FList[start].Index);
	if ((Pozitive==0) && (Negative>0))
		return Indexes.Push(FList[start].Index);
	if (Negative>0)
		return Indexes.Push(firstNegative);

	return false;
}
bool UniqueFeatureConnector::OnInitConnectionToConnector()
{
	GML::ML::MLRecord	rec;	
	UInt32				start,tr,featSize;

	if (conector->CreateMlRecord(rec)==false)
	{
		notifier->Error("[%s] -> Unable to create record from child connector!",ObjectName);
		return false;
	}
	if (Indexes.Create(conector->GetRecordCount())==false)
	{
		notifier->Error("[%s] -> Unable to allocate %d indexes ",ObjectName,conector->GetRecordCount());
		return false;
	}
	if (FList.Create(conector->GetRecordCount(),true)==false)
	{
		notifier->Error("[%s] -> Unable to allocate memory for !",ObjectName);
		return false;
	}
	for (tr=0;tr<FList.Len();tr++)
		FList[tr].Index = tr;

	featSize = conector->GetFeatureCount() * sizeof(double);
	notifier->StartProcent("[%s] -> Computing hashes ... ",ObjectName);
	for (tr=0;tr<FList.Len();tr++)
	{
		if (conector->GetRecord(rec,tr)==false)
		{
			notifier->Error("[%s] -> Unable to read record #%d!",ObjectName,tr);
			return false;
		}
		if (FList[tr].Hash.ComputeHashForBuffer(rec.Features,featSize)==false)
		{
			notifier->Error("[%s] -> Unable to compute hash for record #%d!",ObjectName,tr);
			return false;
		}
		if ((tr % 1000)==0)
			notifier->SetProcent(tr,FList.Len());
	}
	notifier->EndProcent();
	// am citit toate datele , le sortez
	notifier->Info("[%s] -> Sorting ... ",ObjectName);
	FList.Sort(FeatInfoCompare);
	// caut duplicate
	for (tr=start=0;tr<FList.Len();tr++)
	{
		if (memcmp(FList[tr].Hash.Hash.bValue,FList[tr].Hash.Hash.dwValue,16)!=0)
		{
			if (AnalizeSubList(start,tr)==false)
			{
				notifier->Error("[%s] -> Unable to create unique indexes",ObjectName);
				return false;
			}
			start = tr;
		}
	}
	if (AnalizeSubList(start,tr)==false)
	{
		notifier->Error("[%s] -> Unable to create unique indexes",ObjectName);
		return false;
	}

	nrRecords = Indexes.Len();
	columns.nrFeatures = conector->GetFeatureCount();
	dataMemorySize = nrRecords*sizeof(UInt32);
	return false;
}


bool UniqueFeatureConnector::GetRecordLabel( double &label,UInt32 index )
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}
	return conector->GetRecordLabel(label, (UInt32)Indexes.Get(index));
}
bool UniqueFeatureConnector::GetRecord( GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask)
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}

	return conector->GetRecord(record,(UInt32)Indexes.Get(index),recordMask);
}
bool UniqueFeatureConnector::GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index)
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}

	return conector->GetRecordHash(recHash,(UInt32)Indexes.Get(index));
}

bool UniqueFeatureConnector::CreateMlRecord( GML::ML::MLRecord &record )
{
	if (this->conector)
		return this->conector->CreateMlRecord(record);
	return false;
}
bool UniqueFeatureConnector::FreeMLRecord( GML::ML::MLRecord &record )
{
	if (this->conector)
		return this->conector->FreeMLRecord(record);
	return false;
}
bool UniqueFeatureConnector::Close()
{
	if (this->conector)
		return this->conector->Close();	 
	Indexes.Destroy();
	FList.Free();
	return true;
}
