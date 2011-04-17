#include "HashFilterConnector.h"

HashFilterConnector::HashFilterConnector()
{
	FeatureCount = RecordCount = 0;
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
UInt32 HashFilterConnector::GetRecordCount() 
{
	return RecordCount;	
}
bool   HashFilterConnector::GetRecordLabel( double &label,UInt32 index )
{
	if (index >= RecordCount)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,RecordCount-1);
		return false;
	}
	return conector->GetRecordLabel(label, (UInt32)Indexes.Get(index));
}
UInt32 HashFilterConnector::GetFeatureCount()
{
	return FeatureCount;
}
bool   HashFilterConnector::GetRecord( MLRecord &record,UInt32 index,UInt32 recordMask ) 
{
	if (index >= RecordCount)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,RecordCount-1);
		return false;
	}

	return conector->GetRecord(record,(UInt32)Indexes.Get(index),recordMask);
}
bool   HashFilterConnector::GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index)
{
	if (index >= RecordCount)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,RecordCount-1);
		return false;
	}

	return conector->GetRecordHash(recHash,(UInt32)Indexes.Get(index));
}
bool   HashFilterConnector::GetFeatureName(GML::Utils::GString &str,UInt32 index)
{
	return conector->GetFeatureName(str,index);
}
bool   HashFilterConnector::CreateMlRecord( MLRecord &record )
{
	if (this->conector)
		return this->conector->CreateMlRecord(record);
	return false;
}
bool   HashFilterConnector::SetRecordInterval( UInt32 start, UInt32 end )
{
	return false;
}
bool   HashFilterConnector::OnInitConnectionToConnector() 
{
	notifier->Info("[%s] -> Loading hash list from %s",ObjectName,HashFileName.GetText());
	switch (HashFileType)
	{
		case GML::Algorithm::HASH_FILE_BINARY:
			if (LoadBinaryHashFile()==false)
				return false;
			break;
		default:
			notifier->Error("[%s] -> Unknwon hash file type : %d",ObjectName,HashFileType);
			return false;
	}



	RecordCount = Indexes.Len();
	FeatureCount = conector->GetFeatureCount();
	notifier->Info("[%s] -> Done. Total records = %d , Total Features = %d ",ObjectName,RecordCount,FeatureCount);
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
UInt32 HashFilterConnector::GetTotalRecordCount()
{
	if (conector)
		return conector->GetTotalRecordCount();
	return 0;
}