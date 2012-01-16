#include "FeatureReorderConnector.h"

int HashCompareFunction(FeatureIndex &h1,FeatureIndex &h2)
{
	return memcmp(&h1.Hash,&h2.Hash,sizeof(GML::DB::RecordHash));
}
//========================================================
FeatureReorderConnector::FeatureReorderConnector()
{
	ObjectName = "FeatureReorderConnector";

	LinkPropertyToString("FeatureFileName"			,FeatureFileName		,"","Name of the file with the ordered features.");
}
FeatureReorderConnector::~FeatureReorderConnector()
{
	Indexes.Destroy();
}

bool   FeatureReorderConnector::LoadTextHashFile()
{
	GML::Utils::GString		tempStr,line;
	int						poz,count;
	FeatureIndex			fIndex;

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
	fIndex.Index = 0;
	while (tempStr.CopyNextLine(&line,&poz))
	{
		if (line.Len()!=0)
		{
			if (fIndex.Hash.ComputeHashForText(line.GetText()))
			{				
				if (HashList.Push(fIndex)==false)
				{
					notifier->Error("[%s] -> Unable to add '%s' to name hash list",ObjectName,line.GetText());
					return false;
				}
				fIndex.Index ++;
			}
		}
	}
	notifier->Info("[%s] -> %s loaded ok (%d features)",ObjectName,FeatureFileName.GetText(),HashList.Len());
	return true;
}
bool   FeatureReorderConnector::OnInitConnectionToConnector() 
{
	UInt32					tr,conCount;
	FeatureIndex			featIndex;
	GML::Utils::GString		featName;
	int						fIndex;
	

	notifier->Info("[%s] -> Loading feature name list from %s",ObjectName,FeatureFileName.GetText());
	if (LoadTextHashFile()==false)
		return false;
	notifier->StartProcent("[%s] -> Sorting %d elements ... ",ObjectName,HashList.Len());
	HashList.Sort(HashCompareFunction);

	if (Indexes.Create(HashList.Len())==false)
	{
		notifier->Error("[%s] -> Unable to allocate %d indexes for features names",ObjectName,conector->GetFeatureCount());
		return false;
	}
	for (tr=0;tr<HashList.Len();tr++)
	{
		if (Indexes.Push(0xFFFFFFFF)==false)
		{
			notifier->Error("[%s] -> Unable to add feature index #d to index list",ObjectName,tr);
			return false;		
		}
	}
	// bag in list
	notifier->StartProcent("[%s] -> Filtering ",ObjectName);	
	conCount = conector->GetFeatureCount();
	for (tr=0;tr<conCount;tr++)
	{
		if (conector->GetFeatureName(featName,tr)==false)
		{
			notifier->Error("[%s] -> Unable to read feature name #%d",ObjectName,tr);
			return false;
		}
		if (featIndex.Hash.ComputeHashForText(featName.GetText())==false)
		{
			notifier->Error("[%s] -> Unable to compute hash for feature: %s",ObjectName,featName.GetText());
			return false;
		}
		fIndex = HashList.BinarySearch(featIndex,HashCompareFunction);
		if (fIndex<0)
		{
			notifier->Error("[%s] -> Unable to find feature '%s' in FeaturesList",ObjectName,featName.GetText());
			return false;		
		}
		if (fIndex>=Indexes.Len())
		{
			notifier->Error("[%s] -> Index outside allocation (%d). Max allowed is [0..%d)",ObjectName,fIndex,Indexes.Len());
			return false;		
		}
		Indexes.GetList()[HashList[fIndex].Index] = tr;
		if ((tr % 1000)==0)
			notifier->SetProcent(tr,conCount);
	}
	notifier->EndProcent();
	HashList.Free();
	// verific sa fie toti indexii setati
	for (tr=0;tr<Indexes.Len();tr++)
	{
		if (Indexes.Get(tr)>=Indexes.Len())
		{
			notifier->Error("[%s] -> Index (#d) in ordered list not set. Posible 2 identical features !!!",ObjectName,tr);
			return false;		
		}
	}	
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
bool   FeatureReorderConnector::GetRecordLabel( double &label,UInt32 index )
{
	return conector->GetRecordLabel(label, index);
}

bool   FeatureReorderConnector::GetRecord( MLRecord &record,UInt32 index,UInt32 recordMask ) 
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
bool   FeatureReorderConnector::GetFeatureName(GML::Utils::GString &str,UInt32 index)
{
	return conector->GetFeatureName(str,Indexes.Get(index));
}
bool   FeatureReorderConnector::CreateMlRecord( MLRecord &record )
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
bool   FeatureReorderConnector::FreeMLRecord( MLRecord &record )
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
bool   FeatureReorderConnector::Close()
{
	if (this->conector)
		return this->conector->Close();	 
	return false;
}
