#include "UniqueFeatureConnector.h"

int FeatInfoCompare(FeatInfo &f1,FeatInfo &f2)
{
	return memcmp(f1.Hash.Hash.bValue,f2.Hash.Hash.bValue,16);
}
//============================================================================== 
UniqueFeatureConnector::UniqueFeatureConnector()
{
	ObjectName = "UniqueFeatureConnector";

	LinkPropertyToUInt32("IfUniqeRecordPositive",IfUniqeRecordPositive,ACTION_KEEP,"!!LIST:Keep=0,Remove!!");
	LinkPropertyToUInt32("IfUniqeRecordNegative",IfUniqeRecordNegative,ACTION_KEEP,"!!LIST:Keep=0,Remove!!");
	LinkPropertyToUInt32("IfMultipleRecordsPositive",IfMultipleRecordsPositive,ACTION_KEEP_ONE,"!!LIST:KeepOne=0,KeepAll,RemoveAll!!");
	LinkPropertyToUInt32("IfMultipleRecordsNegative",IfMultipleRecordsNegative,ACTION_KEEP_ONE,"!!LIST:KeepOne=0,KeepAll,RemoveAll!!");
	LinkPropertyToUInt32("IfMultiClassRecords",IsMultiClassRecords,ACTION_MC_KEEP_FIRST_NEGATIVE,"!!LIST:KeepAll=0,RemoveAll,KeepFirstPositive,KeepFirstNegative,KeepFirstPositiveAndNegative,KeepOnlyPositive,KeepOnlyNegative!!");
}
UniqueFeatureConnector::~UniqueFeatureConnector()
{
	Close();
}
bool UniqueFeatureConnector::DoActionOnSingleClass(UInt32 start,UInt32 end,UInt32 ifOne,UInt32 ifMany,bool isPositive)
{
	UInt32	tr;
	// daca am unul singur
	if (start+1==end)
	{
		if (ifOne==ACTION_REMOVE)
			return true;
		if (isPositive)
		{
			countInfo.CombPoz++;
			countInfo.UniqPoz++;
		} else {
			countInfo.CombNeg++;
			countInfo.UniqNeg++;
		}
		return Indexes.Push(FList[start].Index);
	}
	// daca am mai multi
	switch (ifMany)
	{
		case ACTION_KEEP_ONE:
			if (isPositive)
			{
				countInfo.CombPoz+=(end-start);
				countInfo.UniqPoz++;
			} else {
				countInfo.CombNeg+=(end-start);
				countInfo.UniqNeg++;
			}
			return Indexes.Push(FList[start].Index);
		case ACTION_KEEP_ALL:
			if (isPositive)
			{
				countInfo.CombPoz+=(end-start);
				countInfo.UniqPoz+=(end-start);
			} else {
				countInfo.CombNeg+=(end-start);
				countInfo.UniqNeg+=(end-start);
			}
			for (tr=start;tr<end;tr++)
			{
				if (Indexes.Push(FList[tr].Index)==false)
					return false;
			}
			return true;
		case ACTION_REMOVE_ALL:
			return true;
	}
	notifier->Error("[%s] ->Unknown metod for many objects: %d!",ObjectName,ifMany);
	return false;
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
	{
		return DoActionOnSingleClass(start,end,IfUniqeRecordPositive,IfMultipleRecordsPositive,true);
	}
	if ((Pozitive==0) && (Negative>0))
	{
		return DoActionOnSingleClass(start,end,IfUniqeRecordNegative,IfMultipleRecordsNegative,false);
	}
	// sunt pe un multi class
	curent = start;
	switch (IsMultiClassRecords)
	{
		case ACTION_MC_KEEP_ALL:
			countInfo.CombPoz+=Pozitive;
			countInfo.CombNeg+=Negative;
			countInfo.UniqPoz+=Pozitive;
			countInfo.UniqNeg+=Negative;
			for (;curent<end;curent++)
			{
				if (Indexes.Push(FList[curent].Index)==false)
					return false;
			}
			return true;
		case ACTION_MC_REMOVE_ALL:
			return true;
		case ACTION_MC_KEEP_FIRST_POSITIVE:
			countInfo.CombPoz+=Pozitive;
			countInfo.UniqPoz++;
			return Indexes.Push(firstPozitive);
		case ACTION_MC_KEEP_FIRST_NEGATIVE:
			countInfo.CombNeg+=Negative;
			countInfo.UniqNeg++;
			return Indexes.Push(firstNegative);
		case ACTION_MC_KEEP_FIRST_POSITIVE_AND_NEGATIVE:
			countInfo.CombPoz+=Pozitive;
			countInfo.CombNeg+=Negative;
			countInfo.UniqNeg++;
			countInfo.UniqPoz++;
			return (Indexes.Push(firstPozitive)  & Indexes.Push(firstNegative));
		case ACTION_MC_KEEP_ONLY_POSITIVE:			
			for (;curent<end;curent++)
			{
				if (conector->GetRecordLabel(Label,FList[curent].Index)==false)
				{
					notifier->Error("[%s] -> Unable to read label for record #%d!",ObjectName,FList[curent].Index);
					return false;
				}
				if (Label==1.0)
				{
					countInfo.CombPoz+=1;
					countInfo.UniqPoz+=1;
					if (Indexes.Push(FList[curent].Index)==false)
						return false;
				}
			}
			return true;
		case ACTION_MC_KEEP_ONLY_NEGATIVE:
			for (;curent<end;curent++)
			{
				if (conector->GetRecordLabel(Label,FList[curent].Index)==false)
				{
					notifier->Error("[%s] -> Unable to read label for record #%d!",ObjectName,FList[curent].Index);
					return false;
				}
				if (Label!=1.0)
				{
					countInfo.CombNeg+=1;
					countInfo.UniqNeg+=1;
					if (Indexes.Push(FList[curent].Index)==false)
						return false;
				}
			}
			return true;
	}

	notifier->Error("[%s] -> Unknwon method for MultiClassRecords",ObjectName,IsMultiClassRecords);
	return false;
}
bool UniqueFeatureConnector::OnInitConnectionToConnector()
{
	GML::ML::MLRecord	rec;	
	GML::Utils::GString	tmp;
	UInt32				start,tr,featSize,max;
	

	memset(&countInfo,0,sizeof(countInfo));
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
		if (rec.Label==1.0)
			countInfo.TotalPoz++;
		else
			countInfo.TotalNeg++;
		if ((tr % 1000)==0)
			notifier->SetProcent(tr,FList.Len());
	}
	notifier->EndProcent();
	// am citit toate datele , le sortez
	notifier->Info("[%s] -> Sorting ... ",ObjectName);
	FList.Sort(FeatInfoCompare);
	
	// caut duplicate
	max = 0;
	for (tr=start=0;tr<FList.Len();tr++)
	{
		if (memcmp(FList[tr].Hash.Hash.bValue,FList[start].Hash.Hash.BValue,16)!=0)
		{
			if (AnalizeSubList(start,tr)==false)
			{
				notifier->Error("[%s] -> Unable to create unique indexes",ObjectName);
				return false;
			}
			if ((tr-start)>max)
				max = tr-start;
			start = tr;
		}
	}
	if (AnalizeSubList(start,tr)==false)
	{
		notifier->Error("[%s] -> Unable to create unique indexes",ObjectName);
		return false;
	}
	if ((tr-start)>max)
		max = tr-start;

	// vad cate sunt pozitive si negative
	tmp.Set("");tmp.AddFormatedEx("[%{str}] -> Initial       : Positive:%{uint32,G3} , Negative:%{uint32,G3}",ObjectName,countInfo.TotalPoz,countInfo.TotalNeg);
	notifier->Info("%s",tmp.GetText());

	tmp.Set("");tmp.AddFormatedEx("[%{str}] -> Unique        : Positive:%{uint32,G3} , Negative:%{uint32,G3}",ObjectName,countInfo.UniqPoz,countInfo.UniqNeg);
	notifier->Info("%s",tmp.GetText());

	tmp.Set("");tmp.AddFormatedEx("[%{str}] -> Combinations  : Positive:%{uint32,G3} , Negative:%{uint32,G3}",ObjectName,countInfo.CombPoz,countInfo.CombNeg);
	notifier->Info("%s",tmp.GetText());

	
	notifier->Info("[%s] -> Max number of duplicates  : %d",ObjectName,max);
	notifier->Info("[%s] -> Number of records removed : %d",ObjectName,conector->GetRecordCount()-Indexes.Len());
	conector->FreeMLRecord(rec);

	nrRecords = Indexes.Len();
	columns.nrFeatures = conector->GetFeatureCount();
	dataMemorySize = (UInt64)nrRecords*sizeof(UInt32);
	return true;
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
