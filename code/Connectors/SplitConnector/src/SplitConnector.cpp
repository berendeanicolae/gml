#include "SplitConnector.h"

SplitConnector::SplitConnector()
{
	FeatureCount = RecordCount = 0;
	ObjectName = "SplitCacheConnector";
	attrStart = attrEnd = 0;
	Start = End = 0;

	LinkPropertyToUInt32("SplitMode", SplitMode, Percentage, "!!LIST:Percentage=0,Range!!");
	LinkPropertyToUInt32("Start", attrStart, 0, "The start percentage/item to split from\n this can mean Percentage Start or Numeric Start\n zero indexed");
	LinkPropertyToUInt32("End", attrEnd, 1000, "The end percentage/item to split from\n this can mean Percentage Stop or Numeric Stop\nzero indexed, the actual index Stop is not included");
}
SplitConnector::~SplitConnector()
{
	Indexes.Destroy();
}
bool   SplitConnector::CreateIndexList()
{
	UInt32		count;
	
	Indexes.Destroy();
	if (Start==End)
	{
		notifier->Error("[%s] Start and End properties can not be equal",ObjectName);
		return false;
	}
	if (Start<End)
	{
		if (End>=conector->GetRecordCount())
			End = conector->GetRecordCount();
		count = End-Start;
	} else {
		if (Start>=conector->GetRecordCount())
			Start = conector->GetRecordCount();
		count = (conector->GetRecordCount()-Start) + End;
	}
	notifier->Info("[%s] Allocing %d records ",ObjectName,RecordCount);
	if (Indexes.Create(count)==false)	
	{
		notifier->Error("[%s] Unable to alloc %d indexes ",ObjectName,RecordCount);
		return false;
	}
	return true;
}
bool   SplitConnector::AddIndexes()
{
	UInt32	cPoz = Start;
	UInt32	recCount = conector->GetRecordCount();
	
	if (cPoz==recCount)
		cPoz==0;
	while (cPoz!=End)
	{
		if (Indexes.Push(cPoz)==false)
		{
			notifier->Error("[%s] Unable to add index #%d ",ObjectName,cPoz);
			return false;
		}
		cPoz++;
		if (cPoz==recCount)
			cPoz==0;
	}
	return true;
}
UInt32 SplitConnector::GetRecordCount() 
{
	return RecordCount;	
}
bool   SplitConnector::GetRecordLabel( double &label,UInt32 index )
{
	if (index >= RecordCount)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,RecordCount-1);
		return false;
	}
	return conector->GetRecordLabel(label, (UInt32)Indexes.Get(index));
}
UInt32 SplitConnector::GetFeatureCount()
{
	return FeatureCount;
}
bool   SplitConnector::GetRecord( MLRecord &record,UInt32 index ) 
{
	if (index >= RecordCount)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,RecordCount-1);
		return false;
	}

	return conector->GetRecord(record,(UInt32)Indexes.Get(index));
}
bool   SplitConnector::CreateMlRecord( MLRecord &record )
{
	if (this->conector)
		return this->conector->CreateMlRecord(record);
	return false;
}
bool   SplitConnector::SetRecordInterval( UInt32 start, UInt32 end )
{
	return false;
}
bool   SplitConnector::OnInit() 
{
	if (this->conector == NULL)
	{
		notifier->Error("[%s] SplitConnector is an intermediate connector, please provide a lower level connector",ObjectName);
		return false;
	}

	notifier->Info("SplitConnector loading data");

	switch (SplitMode)
	{
		case Percentage:
			if ((attrStart>100) || (attrEnd>100))
			{
				notifier->Error("[%s] Procents have to be in interval [0..100]",ObjectName);
				return false;
			}
			Start = (conector->GetRecordCount()*attrStart)/100;
			End = (conector->GetRecordCount()*attrEnd)/100;
			if (CreateIndexList()==false)
				return false;
			if (AddIndexes()==false)
				return false;
			break;
		case Range:
			Start = attrStart;
			End = attrEnd;
			if (CreateIndexList()==false)
				return false;
			if (AddIndexes()==false)
				return false;
			break;
		default:
			notifier->Error("[%s] Invalid Splitting Mode (%d)",ObjectName,SplitMode);
			return false;
	}
	RecordCount = Indexes.Len();
	FeatureCount = conector->GetFeatureCount();
	notifier->Info("[%s] -> Total records = %d , Total Features = %d ",ObjectName,RecordCount,FeatureCount);
	return true;
}
bool   SplitConnector::FreeMLRecord( MLRecord &record )
{
	if (this->conector)
		return this->conector->FreeMLRecord(record);
	return false;
}
bool   SplitConnector::Close()
{
	if (this->conector)
		return this->conector->Close();	 
	return false;
}
UInt32 SplitConnector::GetTotalRecordCount()
{
	if (conector)
		return conector->GetTotalRecordCount();
	return 0;
}