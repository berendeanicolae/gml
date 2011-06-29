#include "HashFilterConnector.h"

int HashCompareFunction(GML::DB::RecordHash &h1,GML::DB::RecordHash &h2)
{
	return memcmp(&h1,&h2,sizeof(GML::DB::RecordHash));
}
//========================================================
HashFilterConnector::HashFilterConnector()
{
	ObjectName = "HashFilterConnector";

	LinkPropertyToString("HashFileName"				,HashFileName			,"","Name of the file with the record hash list result.");
	LinkPropertyToUInt32("HashFileType"				,HashFileType			,0,"!!LIST:Text=0,Binary!!");
	LinkPropertyToUInt32("FilterMethod"				,FilterMethod			,0,"!!LIST:RemoveHash=0,KeepHash!!");
}
HashFilterConnector::~HashFilterConnector()
{
	Indexes.Destroy();
}
bool   HashFilterConnector::LoadBinaryHashFile()
{
	GML::Utils::File	f;
	char				temp[9];
	UInt32				nrHashes;

	if (f.OpenRead(HashFileName.GetText())==false)
	{
		notifier->Error("[%s] -> Unable to open %s",ObjectName,HashFileName.GetText());
		return true;
	}
	while (true)
	{
		if (f.Read(temp,8)==false)
			break;
		if (memcmp(temp,"HASHLIST",8)!=0)
		{
			notifier->Error("[%s] -> Invalid file format : %s",ObjectName,HashFileName.GetText());
			break;
		}
		if (f.Read(&nrHashes,sizeof(UInt32))==false)
			break;
		if (nrHashes==0)
		{
			notifier->Error("[%s] -> Invalid number of Hashes(%d) in %s",ObjectName,nrHashes,HashFileName.GetText());
			break;
		}
		if (HashList.Create(nrHashes)==false)
		{
			notifier->Error("[%s] -> Unable to alloc %d hashes",ObjectName,nrHashes);
			break;
		}
		if (f.Read(HashList.GetVector(),sizeof(GML::DB::RecordHash)*nrHashes)==false)
			break;
		if (HashList.Resize(nrHashes)==false)
			break;
		f.Close();
		notifier->Info("[%s] -> %s loaded ok (%d hashes)",ObjectName,HashFileName.GetText(),nrHashes);
		return true;
	}
	f.Close();
	notifier->Error("[%s] -> Unable to read all data / Invalid format for %s",ObjectName,HashFileName.GetText());
	return false;
}
bool   HashFilterConnector::LoadTextHashFile()
{
	GML::Utils::GString		tempStr,line;
	int						poz,count;
	GML::DB::RecordHash		rHash;

	if (tempStr.LoadFromFile(HashFileName.GetText())==false)
	{
		notifier->Error("[%s] -> Unable to load %s",ObjectName,HashFileName.GetText());
		return false;
	}
	//numar sa vad cate linii am
	poz = count = 0;
	while (tempStr.CopyNextLine(&line,&poz))
	{
		if (line.Len()==32)
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
		if (line.Len()==32)
		{
			if (rHash.CreateFromText(line.GetText()))
			{
				if (HashList.Push(rHash)==false)
				{
					notifier->Error("[%s] -> Unable to add '%s' to hash list",ObjectName,line.GetText());
					return false;
				}
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

bool   HashFilterConnector::GetRecordLabel( double &label,UInt32 index )
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}
	return conector->GetRecordLabel(label, (UInt32)Indexes.Get(index));
}

bool   HashFilterConnector::GetRecord( MLRecord &record,UInt32 index,UInt32 recordMask ) 
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}

	return conector->GetRecord(record,(UInt32)Indexes.Get(index),recordMask);
}
bool   HashFilterConnector::GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index)
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}

	return conector->GetRecordHash(recHash,(UInt32)Indexes.Get(index));
}

bool   HashFilterConnector::CreateMlRecord( MLRecord &record )
{
	if (this->conector)
		return this->conector->CreateMlRecord(record);
	return false;
}

bool   HashFilterConnector::OnInitConnectionToConnector() 
{
	UInt32					tr,conCount;
	GML::DB::RecordHash		recHash;
	bool					isInList;

	notifier->Info("[%s] -> Loading hash list from %s",ObjectName,HashFileName.GetText());
	switch (HashFileType)
	{
		case GML::Algorithm::HASH_FILE_TEXT:
			if (LoadTextHashFile()==false)
				return false;
			break;
		case GML::Algorithm::HASH_FILE_BINARY:
			if (LoadBinaryHashFile()==false)
				return false;
			break;
		default:
			notifier->Error("[%s] -> Unknwon hash file type : %d",ObjectName,HashFileType);
			return false;
	};
	if (Indexes.Create(conector->GetRecordCount())==false)
	{
		notifier->Error("[%s] -> Unable to allocate %d indexes ",ObjectName,conector->GetRecordCount());
		return false;
	}

	// sortare
	notifier->StartProcent("[%s] -> Sorting %d elements ... ",ObjectName,HashList.Len());
	HashList.Sort(HashCompareFunction);
	notifier->EndProcent();
	// bag in list
	notifier->StartProcent("[%s] -> Filtering ",ObjectName);
	conCount = conector->GetRecordCount();
	for (tr=0;tr<conCount;tr++)
	{
		if (conector->GetRecordHash(recHash,tr)==false)
		{
			notifier->Error("[%s] -> Unable to read hash for record #d ",ObjectName,tr);
			return false;
		}
		isInList = (bool)(HashList.BinarySearch(recHash,HashCompareFunction)>=0);
		if ((FilterMethod==FILTER_REMOVE_HASH) && (isInList))
			continue;
		if ((FilterMethod==FILTER_KEEP_HASH) && (!isInList))
			continue;

		//GML::Utils::GString	tmpStr;
		//recHash.ToString(tmpStr);
		//notifier->Info("[%s] -> Adding : %s",ObjectName,tmpStr.GetText());
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

	nrRecords = Indexes.Len();
	columns.nrFeatures = conector->GetFeatureCount();
	dataMemorySize = nrRecords*sizeof(UInt32);
	return true;
}
bool   HashFilterConnector::FreeMLRecord( MLRecord &record )
{
	if (this->conector)
		return this->conector->FreeMLRecord(record);
	return false;
}
bool   HashFilterConnector::Close()
{
	if (this->conector)
		return this->conector->Close();	 
	return false;
}
