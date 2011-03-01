#include "SplitConnector.h"

SplitConnector::SplitConnector()
{
	RecordCount = 0;
	FeatureCount = 0;
	RecordIndexCache = NULL;

	LinkPropertyToUInt32("SplitMode", SplitMode, Percentage, "!!LIST:Percentage=0,Range!!");
	LinkPropertyToUInt32("Start", Start, 0, "The start percentage/item to split from\n this can mean Percentage Start or Numeric Start\n zero indexed");
	LinkPropertyToUInt32("Stop", Stop, 0, "The stop percentage/item to split from\n this can mean Percentage Stop or Numeric Stop\nzero indexed, the actual index Stop is not included");
}

SplitConnector::~SplitConnector()
{
	if (RecordIndexCache)
		delete RecordIndexCache;
}

UInt32 SplitConnector::GetRecordCount() 
{
	return RecordCount;	
}

bool SplitConnector::GetRecordLabel( double &label,UInt32 index )
{
	if (index >= RecordCount)
	{
		notifier->Error("index out of range, the maximum allowed is %d",RecordCount-1);
		return false;
	}
	return conector->GetRecordLabel(label, (UInt32)RecordIndexCache[index]);
}

UInt32 SplitConnector::GetFeatureCount()
{
	if (this->conector)
		return conector->GetFeatureCount();
	return 0;
}

bool SplitConnector::GetRecord( MLRecord &record,UInt32 index ) 
{
	if (index >= RecordCount)
	{
		notifier->Error("index out of range, the maximum allowed is %d",RecordCount-1);
		return false;
	}

	return conector->GetRecord(record,(UInt32) RecordIndexCache[index]);	
}

bool SplitConnector::CreateMlRecord( MLRecord &record )
{
	if (this->conector)
		return this->conector->CreateMlRecord(record);
	return false;
}

bool SplitConnector::SetRecordInterval( UInt32 start, UInt32 end )
{
	return false;
}

bool SplitConnector::OnInitPercentage() 
{
	if (Stop >100 || Start > Stop)
	{
		notifier->Error("Start or Stop are not in the accepted ranges");
		return false;
	}

	TotalRecordCount = this->conector->GetRecordCount();
	FeatureCount = this->conector->GetFeatureCount();
	RecordCount = (Stop-Start)/100 * TotalRecordCount;

	RecordIndexCache = new double[RecordCount];
	if (RecordIndexCache == NULL)
	{
		notifier->Error("could not allocate memory");
		return false;
	}

	UInt32 startIndex, stopIndex;
	startIndex = Start/100 * TotalRecordCount;
	stopIndex  = Stop/100 * TotalRecordCount;

	for (UInt32 tr=startIndex;tr<stopIndex;tr++)
		RecordIndexCache[tr-startIndex] = tr;

	return true;
}

bool SplitConnector::OnInitRange() 
{	
	TotalRecordCount = this->conector->GetRecordCount();
	FeatureCount = this->conector->GetFeatureCount();
	RecordCount = Stop-Start;

	if (Stop >TotalRecordCount || Start > Stop)
	{
		notifier->Error("Start or Stop are not in the accepted ranges");
		return false;
	}

	RecordIndexCache = new double[RecordCount];
	if (RecordIndexCache == NULL)
	{
		notifier->Error("could not allocate memory");
		return false;
	}	

	for (UInt32 tr=Start;tr<Stop;tr++)
		RecordIndexCache[tr-Start] = tr;

	return true;
}

bool SplitConnector::OnInit() 
{
	if (this->conector == NULL)
	{
		notifier->Error("SplitConnector is an intermediate connector, please provide a lower level connector");
		return false;
	}

	switch (SplitMode)
	{
	case Percentage:
		return OnInitPercentage();
	case Range:
		return OnInitRange();
	default:
		return false;
	}

	return true;
}

bool SplitConnector::FreeMLRecord( MLRecord &record )
{
	if (this->conector)
		return this->conector->FreeMLRecord(record);
	return false;
}

bool SplitConnector::Close()
{
	if (this->conector)
		return this->conector->Close();	 
	return false;
}

UInt32 SplitConnector::GetTotalRecordCount()
{
	return TotalRecordCount;
}