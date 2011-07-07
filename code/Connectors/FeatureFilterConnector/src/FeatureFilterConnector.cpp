#include "FeatureFilterConnector.h"

int HashCompareFunction(GML::DB::RecordHash &h1,GML::DB::RecordHash &h2)
{
	return memcmp(&h1,&h2,sizeof(GML::DB::RecordHash));
}
//========================================================
FeatureFilterConnector::FeatureFilterConnector()
{
	ObjectName = "FeatureFilterConnector";

	LinkPropertyToString("FeatureFileName"			,FeatureFileName		,"","Name of the file with the record hash list result.");
	LinkPropertyToUInt32("FilterMethod"				,FilterMethod			,0,"!!LIST:RemoveHash=0,KeepHash!!");
}
FeatureFilterConnector::~FeatureFilterConnector()
{
	Indexes.Destroy();
}

bool   FeatureFilterConnector::LoadTextHashFile()
{
	GML::Utils::GString		tempStr,line;
	int						poz,count;
	GML::DB::RecordHash		rHash;

	if (tempStr.LoadFromFile(FeatureFileName.GetText())==false)
	{
		notifier->Error("[%s] -> Unable to load %s",ObjectName,FeatureFileName.GetText());
		return false;
	}
	//numar sa vad cate linii am
	poz = count = 0;
	while (tempStr.CopyNextLine(&line,&poz))
	{
		if (line.Len()!=0)
			count++;
	}
	if (count==0)
	{
		notifier->Error("[%s] -> No feature names found in %s",ObjectName,FeatureFileName.GetText());
		return false;
	}
	if (HashList.Create(count)==false)
	{
		notifier->Error("[%s] -> Unable to alloc %d name hashes",ObjectName,count);
		return false;
	}
	poz = 0;
	while (tempStr.CopyNextLine(&line,&poz))
	{
		if (line.Len()!=0)
		{
			if (rHash.ComputeHashForText(line.GetText()))
			{
				if (HashList.Push(rHash)==false)
				{
					notifier->Error("[%s] -> Unable to add '%s' to name hash list",ObjectName,line.GetText());
					return false;
				}
			}
		}
	}
	notifier->Info("[%s] -> %s loaded ok (%d features)",ObjectName,FeatureFileName.GetText(),HashList.Len());
	return true;
}
bool   FeatureFilterConnector::OnInitConnectionToConnector() 
{
	UInt32					tr,conCount;
	GML::DB::RecordHash		recHash;
	GML::Utils::GString		featName;
	bool					isInList;

	notifier->Info("[%s] -> Loading feature name list from %s",ObjectName,FeatureFileName.GetText());
	if (LoadTextHashFile()==false)
		return false;
	notifier->StartProcent("[%s] -> Sorting %d elements ... ",ObjectName,HashList.Len());
	HashList.Sort(HashCompareFunction);

	if (Indexes.Create(conector->GetFeatureCount())==false)
	{
		notifier->Error("[%s] -> Unable to allocate %d indexes for features names",ObjectName,conector->GetFeatureCount());
		return false;
	}

	// bag in list
	notifier->StartProcent("[%s] -> Filtering ",ObjectName);
	conCount = conector->GetRecordCount();
	for (tr=0;tr<conCount;tr++)
	{
		if (conector->GetFeatureName(featName,tr)==false)
		{
			notifier->Error("[%s] -> Unable to read feature name #%d",ObjectName,tr);
			return false;
		}
		if (recHash.ComputeHashForText(featName.GetText())==false)
		{
			notifier->Error("[%s] -> Unable to compute hash for feature: %s",ObjectName,featName.GetText());
			return false;
		}
		isInList = (bool)(HashList.BinarySearch(recHash,HashCompareFunction)>=0);
		if ((FilterMethod==FILTER_REMOVE_FEATURE) && (isInList))
			continue;
		if ((FilterMethod==FILTER_KEEP_FEATURE) && (!isInList))
			continue;

		if (Indexes.Push(tr)==false)
		{
			notifier->Error("[%s] -> Unable to add record #d to index list",ObjectName,tr);
			return false;
		}
		if ((tr % 1000)==0)
			notifier->SetProcent(tr,conCount);
	}
	notifier->EndProcent();
	HashList.Free();

	if (Indexes.Len()==0)
	{
		notifier->Error("[%s] -> No features found ... exiting ",ObjectName);
		return false;
	}

	nrRecords = conector->GetRecordCount();
	columns.nrFeatures = Indexes.Len();
	dataMemorySize = Indexes.Len()*sizeof(UInt32);
	return true;
}
bool   FeatureFilterConnector::GetRecordLabel( double &label,UInt32 index )
{
	return conector->GetRecordLabel(label, index);
}

bool   FeatureFilterConnector::GetRecord( MLRecord &record,UInt32 index,UInt32 recordMask ) 
{
	UInt32	*idx = Indexes.GetList();

	if (conector->GetRecord(*record.Parent,index,recordMask)==false)
		return false;
	
	record.Label = record.Parent->Label;
	record.FeatCount = columns.nrFeatures;
	if (recordMask & GML::ML::ConnectorFlags::STORE_HASH)
		record.Hash.Copy(record.Parent->Hash);
	
	for (UInt32 tr=0;tr<columns.nrFeatures;tr++,idx++)
		record.Features[tr] = record.Parent->Features[*idx];

	return true;
}
bool   FeatureFilterConnector::GetFeatureName(GML::Utils::GString &str,UInt32 index)
{
	return conector->GetFeatureName(str,Indexes.Get(index));
}
bool   FeatureFilterConnector::CreateMlRecord( MLRecord &record )
{
	if ((record.Parent = new GML::ML::MLRecord())==NULL)
	{
		notifier->Error("[%s] -> Unable to create MLRecord ",ObjectName);
		return false;
	}
	if (conector->CreateMlRecord(*record.Parent)==false)
	{
		notifier->Error("[%s] -> Unable to create MLRecord from parent",ObjectName);
		return false;
	}
	record.FeatCount = columns.nrFeatures;	
	return ((record.Features = new double[columns.nrFeatures])!=NULL);
}
bool   FeatureFilterConnector::FreeMLRecord( MLRecord &record )
{
	if (record.Features!=NULL)
	{
		delete record.Features;
		record.Features = NULL;
	}
	if (conector->FreeMLRecord(*record.Parent))
		return false;
	return true;
}
bool   FeatureFilterConnector::Close()
{
	if (this->conector)
		return this->conector->Close();	 
	return false;
}
