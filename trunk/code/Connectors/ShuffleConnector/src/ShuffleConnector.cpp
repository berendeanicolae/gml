#include "ShuffleConnector.h"
#include "math.h"

//========================================================
ShuffleConnector::ShuffleConnector()
{
	ObjectName = "ShuffleConnector";

	LinkPropertyToUInt32("Method"				,Method			,SHUFFLE_METHOD_RANDOM,"!!LIST:PozitiveFirst,NegativeFirst,Random,Uniform!!");
}
ShuffleConnector::~ShuffleConnector()
{
	Indexes.Destroy();
}


bool   ShuffleConnector::GetRecordLabel( double &label,UInt32 index )
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}
	return conector->GetRecordLabel(label, (UInt32)Indexes.Get(index));
}
bool   ShuffleConnector::GetRecord( MLRecord &record,UInt32 index,UInt32 recordMask ) 
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}

	return conector->GetRecord(record,(UInt32)Indexes.Get(index),recordMask);
}
bool   ShuffleConnector::GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index)
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}

	return conector->GetRecordHash(recHash,(UInt32)Indexes.Get(index));
}

bool   ShuffleConnector::CreateMlRecord( MLRecord &record )
{
	if (this->conector)
		return this->conector->CreateMlRecord(record);
	return false;
}

bool   ShuffleConnector::ShufflePozitiveFirst()
{
	UInt32	tr;
	double  label;

	notifier->Info("[%s] -> Ordering (First pozitive)",ObjectName);
	// adaug cele pozitive
	for (tr=0;tr<conector->GetRecordCount();tr++)
	{
		if (conector->GetRecordLabel(label,tr)==false)
		{
			notifier->Error("[%s] -> Unable to read label for %d index ",ObjectName,tr);
			return false;
		}
		if (label==1)
		{
			if (Indexes.Push(tr)==false)
			{
				notifier->Error("[%s] -> Unable to add record #d to index list",ObjectName,tr);
				return false;
			}
		}
	}
	
	// adaug cele negative
	for (tr=0;tr<conector->GetRecordCount();tr++)
	{
		if (conector->GetRecordLabel(label,tr)==false)
		{
			notifier->Error("[%s] -> Unable to read label for %d index ",ObjectName,tr);
			return false;
		}
		if (label!=1)
		{
			if (Indexes.Push(tr)==false)
			{
				notifier->Error("[%s] -> Unable to add record #d to index list",ObjectName,tr);
				return false;
			}
		}
	}
	return true;
}
bool   ShuffleConnector::ShuffleNegativeFirst()
{
	UInt32	tr;
	double  label;

	notifier->Info("[%s] -> Ordering (First negative)",ObjectName);
	
	for (tr=0;tr<conector->GetRecordCount();tr++)
	{
		if (conector->GetRecordLabel(label,tr)==false)
		{
			notifier->Error("[%s] -> Unable to read label for %d index ",ObjectName,tr);
			return false;
		}
		if (label!=1)
		{
			if (Indexes.Push(tr)==false)
			{
				notifier->Error("[%s] -> Unable to add record #d to index list",ObjectName,tr);
				return false;
			}
		}
	}
		
	for (tr=0;tr<conector->GetRecordCount();tr++)
	{
		if (conector->GetRecordLabel(label,tr)==false)
		{
			notifier->Error("[%s] -> Unable to read label for %d index ",ObjectName,tr);
			return false;
		}
		if (label==1)
		{
			if (Indexes.Push(tr)==false)
			{
				notifier->Error("[%s] -> Unable to add record #d to index list",ObjectName,tr);
				return false;
			}
		}
	}
	return true;
}
bool   ShuffleConnector::ShuffleRandom()
{
	UInt32	tr,i1,i2,aux;
	UInt32	*idx;

	notifier->Info("[%s] -> Ordering (random order)",ObjectName);
	
	for (tr=0;tr<conector->GetRecordCount();tr++)
	{
		if (Indexes.Push(tr)==false)
		{
			notifier->Error("[%s] -> Unable to add record #d to index list",ObjectName,tr);
			return false;
		}		
	}
	
	srand(GetTickCount());
	idx = Indexes.GetList();
	for (tr=0;tr<conector->GetRecordCount();tr++)
	{
		i1 = rand() % conector->GetRecordCount();
		i2 = rand() % conector->GetRecordCount();
		if (i1!=i2)
		{
			aux = idx[i1];
			idx[i1] = idx[i2];
			idx[i2] = aux;
		}
	}
	return true;
}
bool   ShuffleConnector::ShuffleUniform()
{
	UInt32	tr,countPozitive,countNegative;
	double  label,rapPoz,rapNeg;

	notifier->Info("[%s] -> Ordering (uniform distribution)",ObjectName);
	
	countPozitive = countNegative = 0;
	for (tr=0;tr<conector->GetRecordCount();tr++)
	{
		if (conector->GetRecordLabel(label,tr)==false)
		{
			notifier->Error("[%s] -> Unable to read label for %d index ",ObjectName,tr);
			return false;
		}
		if (label==1)
			countPozitive++;
		else
			countNegative++;
	}
	notifier->Info("[%s] -> Pozitive elements = %d, Negative elements = %d",ObjectName,countPozitive,countNegative);
	if ((countPozitive==0) || (countNegative==0))
	{
		notifier->Error("[%s] -> There must be at least one element in each class (negative and pozitive)",ObjectName);
		return false;
	}
	// adaug cele negative
	rapPoz = rapNeg = 1;
	if (countPozitive>countNegative)
		rapNeg = ((double)countPozitive)/((double)countNegative);
	if (countPozitive<countNegative)
		rapPoz = ((double)countNegative)/((double)countPozitive);
	// to do ...

	return true;
}
bool   ShuffleConnector::OnInitConnectionToConnector() 
{
	if (Indexes.Create(conector->GetRecordCount())==false)
	{
		notifier->Error("[%s] -> Unable to allocate %d indexes ",ObjectName,conector->GetRecordCount());
		return false;
	}

	switch (Method)
	{
		case SHUFFLE_METHOD_POZITIVE_FIRST:
			if (ShufflePozitiveFirst()==false)
				return false;
			break;
		case SHUFFLE_METHOD_NEGATIVE_FIRST:
			if (ShuffleNegativeFirst()==false)
				return false;
			break;
		case SHUFFLE_METHOD_RANDOM:
			if (ShuffleRandom()==false)
				return false;
			break;
		case SHUFFLE_METHOD_UNIFORM:
			notifier->Error("[%s] -> Uniform method not implemented yet !!!",ObjectName);
			return false;
			break;
		default:
			notifier->Error("[%s] -> Unknwon shuffle method (%d)",ObjectName,Method);
			return false;
	}


	nrRecords = Indexes.Len();
	columns.nrFeatures = conector->GetFeatureCount();
	dataMemorySize = nrRecords*sizeof(UInt32);
	return true;
}
bool   ShuffleConnector::FreeMLRecord( MLRecord &record )
{
	if (this->conector)
		return this->conector->FreeMLRecord(record);
	return false;
}
bool   ShuffleConnector::Close()
{
	if (this->conector)
		return this->conector->Close();	 
	return false;
}
