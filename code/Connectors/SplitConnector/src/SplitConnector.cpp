#include "SplitConnector.h"

SplitConnector::SplitConnector()
{
	ObjectName = "SplitConnector";
	attrStart = attrEnd = 0;
	Start = End = 0;

	LinkPropertyToUInt32("SplitMode",		SplitMode, Percentage,	"!!LIST:Percentage=0,Range,UniformPercentage,CustomPercentage!!");
	LinkPropertyToUInt32("Start",			attrStart, 0,			"The start percentage/item to split from\n this can mean Percentage Start or Numeric Start\n zero indexed");
	LinkPropertyToUInt32("End",				attrEnd, 100,			"The end percentage/item to split from\n this can mean Percentage End or Numeric End\nzero indexed, the actual index End is not included");
	LinkPropertyToUInt32("PozitiveStart",	pozitiveProcStart,0,	"The start procent for pozitive Class (works only for SplitMode = CustomPercentage)");
	LinkPropertyToUInt32("PozitiveEnd",		pozitiveProcEnd,100,	"The end procent for pozitive Class (works only for SplitMode = CustomPercentage)");
	LinkPropertyToUInt32("NegativeStart",	negativeProcStart,0,	"The start procent for negative Class (works only for SplitMode = CustomPercentage)");
	LinkPropertyToUInt32("NegativeEnd",		negativeProcEnd,100,	"The end procent for negative Class (works only for SplitMode = CustomPercentage)");
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
	notifier->Info("[%s] Allocing %d records ",ObjectName,count);
	if (Indexes.Create(count)==false)	
	{
		notifier->Error("[%s] Unable to alloc %d indexes ",ObjectName,count);
		return false;
	}
	return true;
}
bool   SplitConnector::AddIndexes()
{
	UInt32	cPoz = Start;
	UInt32	recCount = conector->GetRecordCount();
	
	if (cPoz==recCount)
		cPoz=0;
	while (cPoz!=End)
	{
		if (Indexes.Push(cPoz)==false)
		{
			notifier->Error("[%s] Unable to add index #%d ",ObjectName,cPoz);
			return false;
		}
		cPoz++;
		if ((cPoz==recCount) && (cPoz!=End))
			cPoz=0;
	}
	return true;
}
bool   SplitConnector::CheckProcentInterval(UInt32	pStart,UInt32 pEnd)
{
	if (pStart>100)
	{
		notifier->Error("[%s] -> Invalid procent value %d ",ObjectName,pStart);
		return false;
	}
	if (pEnd>100)
	{
		notifier->Error("[%s] -> Invalid procent value %d ",ObjectName,pEnd);
		return false;
	}
	return true;
}
bool   SplitConnector::CreateUniformPercentageIndex(UInt32 pozStart,UInt32 pozEnd,UInt32 negStart,UInt32 negEnd)
{
	UInt32		countByClass[2]={0,0};
	UInt32		_start[2];
	UInt32		_end[2];
	UInt32		_count[2];
	UInt32		_added[2];
	UInt32		tr,rCount,ID;
	double		label;
	bool		add;
	// numaram cate sunt

	Indexes.Destroy();
	
	if (CheckProcentInterval(pozStart,pozEnd)==false)
		return false;
	if (CheckProcentInterval(negStart,negEnd)==false)
		return false;

	rCount = conector->GetRecordCount();
	for (tr=0;tr<rCount;tr++)
	{
		if (conector->GetRecordLabel(label,tr)==false)
		{
			notifier->Error("[%s] -> Unable to read label for ",ObjectName);
			return false;
		}
		if (label==1)
			countByClass[1]++;
		else
			countByClass[0]++;
	}

	notifier->Info("[%s] -> Pozitive = %d, Negative = %d",ObjectName,countByClass[1],countByClass[0]);
	for (tr=0;tr<2;tr++)
	{
		if (tr==0)
		{
			_start[tr]=(countByClass[tr]*negStart)/100;
			_end[tr]=(countByClass[tr]*negEnd)/100;
		} else {
			_start[tr]=(countByClass[tr]*pozStart)/100;
			_end[tr]=(countByClass[tr]*pozEnd)/100;
		}
		if (_start[tr]<_end[tr])
			_count[tr] = _end[tr]-_start[tr];
		else
		if (_start[tr]>_end[tr])
			_count[tr] = (countByClass[tr]-_start[tr])+_end[tr];
		else
			_count[tr]=0;
	}
	if ((_count[0]+_count[1])==0)
	{
		notifier->Error("[%s] -> To few elements (0 elements)",ObjectName);
		return false;
	}
	notifier->Info("[%s] -> Allocing %d records (Poz:%d%% to %d%%) and (Neg:%d%% to %d%%)",ObjectName,_count[0]+_count[1],pozStart,pozEnd,negStart,negEnd);
	if (Indexes.Create(_count[0]+_count[1])==false)	
	{
		notifier->Error("[%s] -> Unable to alloc %d indexes ",ObjectName,_count[0]+_count[1]);
		return false;
	}
	// adaug si elementele
	_count[0]=_count[1]=0;
	_added[0]=_added[1]=0;
	for (tr=0;tr<rCount;tr++)
	{
		if (conector->GetRecordLabel(label,tr)==false)
		{
			notifier->Error("[%s] -> Unable to read label for ",ObjectName);
			return false;
		}
		if (label==1)
			ID = 1;
		else
			ID = 0;
		add = false;
		if (_start[ID]<_end[ID])
		{
			if ((_count[ID]>=_start[ID]) && (_count[ID]<_end[ID]))
				add = true;
		}
		if (_start[ID]>_end[ID])
		{
			if ((_count[ID]>=_start[ID]) || (_count[ID]<_end[ID]))
				add = true;
		}
		_count[ID]++;
		if (add)
		{
			if (Indexes.Push(tr)==false)
			{
				notifier->Error("[%s] -> Unable to add index #%d ",ObjectName,tr);
				return false;
			}
			_added[ID]++;
		}
	}
	notifier->Info("[%s] -> Added: Pozitive = %d, Negative = %d",ObjectName,_added[1],_added[0]);
	return true;
}

bool   SplitConnector::GetRecordLabel( double &label,UInt32 index )
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}
	return conector->GetRecordLabel(label, (UInt32)Indexes.Get(index));
}

bool   SplitConnector::GetRecord( MLRecord &record,UInt32 index,UInt32 recordMask ) 
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}

	return conector->GetRecord(record,(UInt32)Indexes.Get(index),recordMask);
}
bool   SplitConnector::GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index)
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}

	return conector->GetRecordHash(recHash,(UInt32)Indexes.Get(index));
}

bool   SplitConnector::CreateMlRecord( MLRecord &record )
{
	if (this->conector)
		return this->conector->CreateMlRecord(record);
	return false;
}

bool   SplitConnector::OnInitConnectionToConnector() 
{
	notifier->Info("[%s] -> Loading data (Start = %d,End = %d)",ObjectName,attrStart,attrEnd);

	switch (SplitMode)
	{
		case Percentage:
			if (CheckProcentInterval(attrStart,attrEnd)==false)
				return false;
			Start = (conector->GetRecordCount()*attrStart)/100;
			End = (conector->GetRecordCount()*attrEnd)/100;
			if (CreateIndexList()==false)
				return false;
			if (AddIndexes()==false)
				return false;
			break;
		case UniformPercentage:
			if (CreateUniformPercentageIndex(attrStart,attrEnd,attrStart,attrEnd)==false)
				return false;
			break;
		case CustomPercentage:
			if (CreateUniformPercentageIndex(pozitiveProcStart,pozitiveProcEnd,negativeProcStart,negativeProcEnd)==false)
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
			notifier->Error("[%s] -> Invalid Splitting Mode (%d)",ObjectName,SplitMode);
			return false;
	}
	nrRecords = Indexes.Len();
	columns.nrFeatures = conector->GetFeatureCount();
	dataMemorySize = nrRecords*sizeof(UInt32);
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
