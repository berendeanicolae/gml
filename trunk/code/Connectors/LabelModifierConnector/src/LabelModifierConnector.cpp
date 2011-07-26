#include "LabelModifierConnector.h"

int HashCompareFunction(LabelInfo &h1,LabelInfo &h2)
{
	return memcmp(&h1.Hash,&h2.Hash,sizeof(GML::DB::RecordHash));
}

LabelModifierConnector::LabelModifierConnector()
{
	ObjectName = "LabelModifierConnector";
	
	LinkPropertyToString("HashFileName"				,HashFileName			,"","Name of the file with the record hash and labels");
}
bool   LabelModifierConnector::LoadTextHashFile()
{
	GML::Utils::GString		tempStr,line,l,r;
	int						poz,count;
	LabelInfo				li;

	if (tempStr.LoadFromFile(HashFileName.GetText())==false)
	{
		notifier->Error("[%s] -> Unable to load %s",ObjectName,HashFileName.GetText());
		return false;
	}
	//numar sa vad cate linii am
	poz = count = 0;
	while (tempStr.CopyNextLine(&line,&poz))
	{
		line.Strip();
		if (line.Len()>33)
			count++;
	}
	if (count==0)
	{
		notifier->Error("[%s] -> No hashes found in %s",ObjectName,HashFileName.GetText());
		return false;
	}
	if (HashList.Create(count)==false)
	{
		notifier->Error("[%s] -> Unable to alloc %d hashes",ObjectName,count);
		return false;
	}
	poz = 0;
	while (tempStr.CopyNextLine(&line,&poz))
	{
		line.Strip();
		if ((line.Len()>32) && (line.Contains(":")))
		{
			
			if (line.SplitInTwo(":",&l,&r)==false)
			{
				notifier->Error("[%s] -> Invalid format in line: %s",ObjectName,line.GetText());
				return false;			
			}
			if (li.Hash.CreateFromText(l.GetText())==false)
			{
				notifier->Error("[%s] -> Invalid md5 format in line: %s (md5='%s')",ObjectName,line.GetText(),l.GetText());
				return false;			
			}
			if (r.ConvertToDouble(&li.Label)==false)
			{
				notifier->Error("[%s] -> Invalid label format in line: %s (Label='%s')",ObjectName,line.GetText(),r.GetText());
				return false;				
			}
			if (HashList.PushByRef(li)==false)
			{
				notifier->Error("[%s] -> Unable to add '%s' to hash list",ObjectName,line.GetText());
				return false;
			}
		}
	}
	if (HashList.Len()==0)
	{
		notifier->Error("[%s] -> No hashes found in %s",ObjectName,HashFileName.GetText());
		return false;
	}
	notifier->Info("[%s] -> %s loaded ok (%d hashes)",ObjectName,HashFileName.GetText(),HashList.Len());
	return true;
}

bool LabelModifierConnector::OnInitConnectionToConnector()
{
	UInt32					tr,conCount;
	LabelInfo				lInfo;
	int						index;
	bool					isInList;
	double					Label;

	notifier->Info("[%s] -> Loading hash list and labels from %s",ObjectName,HashFileName.GetText());
	if (LoadTextHashFile()==false)
		return false;	
	// sortare
	HashList.Sort(HashCompareFunction);
	if (Labels.Create(conector->GetRecordCount(),true)==false)
	{
		notifier->Error("[%s] -> Unable to alloc memory for labels ... ",ObjectName);
		return false;	
	}

	// bag in list
	notifier->StartProcent("[%s] -> Filtering ",ObjectName);
	conCount = conector->GetRecordCount();
	for (tr=0;tr<conCount;tr++)
	{
		if (conector->GetRecordHash(lInfo.Hash,tr)==false)
		{
			notifier->Error("[%s] -> Unable to read hash for record #d ",ObjectName,tr);
			return false;
		}
		index = HashList.BinarySearch(lInfo,HashCompareFunction);
		if (index>=0)
		{
			Labels[tr] = HashList[index].Label;
		} else {
			if (conector->GetRecordLabel(Label,tr)==false)
			{
				notifier->Error("[%s] -> Unable to read label for record #d ",ObjectName,tr);
				return false;
			}		
			Labels[tr] = Label;
		}
		if ((tr % 1000)==0)
			notifier->SetProcent(tr,conCount);
	}
	notifier->EndProcent();
	HashList.Free();

	nrRecords = HashList.Len();
	columns.nrFeatures = conector->GetFeatureCount();
	dataMemorySize = (UInt64)nrRecords*sizeof(UInt32);
	return true;
}
bool LabelModifierConnector::GetRecordLabel( double &label,UInt32 index )
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}
	label = Labels[index];
	return true;
}
bool LabelModifierConnector::GetRecord( GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask)
{
	bool	result;
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}
	result = conector->GetRecord(record,index,recordMask);
	record.Label = Labels[index];
	return result;
}
bool LabelModifierConnector::GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index)
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}

	return conector->GetRecordHash(recHash,index);
}
bool LabelModifierConnector::CreateMlRecord( GML::ML::MLRecord &record )
{
	if (this->conector)
		return this->conector->CreateMlRecord(record);
	return false;
}
bool LabelModifierConnector::FreeMLRecord( GML::ML::MLRecord &record )
{
	if (this->conector)
		return this->conector->FreeMLRecord(record);
	return false;
}

bool LabelModifierConnector::Close()
{
	if (this->conector)
		return this->conector->Close();	 
	return false;
}
