#include "MapConnector.h"

MapConnector::MapConnector()
{
	ObjectName = "MapConnector";

	LinkPropertyToUInt32("MapMethod",mapMethod,UseAND,	"!!LIST:UseAND=0,UseOR,UseXOR,UseAnd+Or,UseMultiply,UseAddition!!\n"
														"Selects the method to be used for mapping\n"
														"* UseAND -> feat(1..n) will become feat`(1..m) where\n"														
														"\n"
														);
}

bool	MapConnector::OnInitConnectionToConnector()
{
	featuresCount = 0;
	// calculez cate feature-uri o sa am in functie de metoda
	switch (mapMethod)
	{
		case UseAND:
		case UseXOR:
		case UseOR:
		case UseMultiply:
		case UseAddition:
			featuresCount = (conector->GetFeatureCount() * (conector->GetFeatureCount()+1))>>1;
			break;
		case UseAnd_Or:
			featuresCount = (conector->GetFeatureCount() * (conector->GetFeatureCount()+1));
			break;
		default:
			notifier->Error("[%s] -> Unknown method ID = %d",ObjectName,mapMethod);
			return false;
	};
	notifier->Info("[%s] -> Mapping %d features to %d features ",ObjectName,conector->GetFeatureCount(),featuresCount);
	return true;
}
bool	MapConnector::Close()
{
	return true;
}
bool	MapConnector::SetRecordInterval(UInt32 start, UInt32 end)
{
	return true;
}
bool	MapConnector::CreateMlRecord (GML::ML::MLRecord &record)
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
	record.FeatCount = featuresCount;
	return ((record.Features = new double[featuresCount])!=NULL);
}
bool	MapConnector::GetRecord(GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask)
{
	UInt32		tr,gr,pCount;
	double		*p1,*p2,*pMap;

	if (conector->GetRecord(*record.Parent,index,recordMask)==false)
		return false;
	pCount = conector->GetFeatureCount();
	record.Weight = record.Parent->Weight;
	record.Label = record.Parent->Label;
	record.FeatCount = featuresCount;
	if (recordMask & GML::ML::RECORD_STORE_HASH)
		record.Hash.Copy(record.Parent->Hash);

	pMap = record.Features;
	switch (mapMethod)
	{
		case UseAND:
			for (tr=0,p1=record.Parent->Features;tr<pCount;tr++,p1++)
				for (gr=tr,p2=p1;gr<pCount;gr++,p2++,pMap++)
					(*pMap) = (double)((UInt32)(*p1) & (UInt32)(*p2));
			break;
		case UseOR:
			for (tr=0,p1=record.Parent->Features;tr<pCount;tr++,p1++)
				for (gr=tr,p2=p1;gr<pCount;gr++,p2++,pMap++)
					(*pMap) = (double)((UInt32)(*p1) | (UInt32)(*p2));
			break;
		case UseXOR:
			for (tr=0,p1=record.Parent->Features;tr<pCount;tr++,p1++)
				for (gr=tr,p2=p1;gr<pCount;gr++,p2++,pMap++)
					(*pMap) = (double)((UInt32)(*p1) ^ (UInt32)(*p2));
			break;
		case UseAnd_Or:
			for (tr=0,p1=record.Parent->Features;tr<pCount;tr++,p1++)
				for (gr=tr,p2=p1;gr<pCount;gr++,p2++,pMap++)
				{
					(*pMap) = (double)((UInt32)(*p1) & (UInt32)(*p2));
					pMap++;
					(*pMap) = (double)((UInt32)(*p1) | (UInt32)(*p2));
				}
			break;
		case UseMultiply:
			for (tr=0,p1=record.Parent->Features;tr<pCount;tr++,p1++)
				for (gr=tr,p2=p1;gr<pCount;gr++,p2++,pMap++)
					(*pMap) = (*p1) * (*p2);
			break;
		case UseAddition:
			for (tr=0,p1=record.Parent->Features;tr<pCount;tr++,p1++)
				for (gr=tr,p2=p1;gr<pCount;gr++,p2++,pMap++)
					(*pMap) = (*p1) + (*p2);
			break;
		default:
			notifier->Error("[%s] -> Unknown method ID = %d",ObjectName,mapMethod);
			return false;
	}

	return true;
}
bool	MapConnector::GetRecordLabel(double &Label,UInt32 index)
{
	return conector->GetRecordLabel(Label,index);
}
bool	MapConnector::GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index)
{
	if (index >= conector->GetRecordCount())
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,conector->GetRecordCount()-1);
		return false;
	}

	return conector->GetRecordHash(recHash,index);
}
bool	MapConnector::FreeMLRecord(GML::ML::MLRecord &record)
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
UInt32	MapConnector::GetFeatureCount()
{
	return featuresCount;
}
UInt32	MapConnector::GetRecordCount()
{
	return conector->GetRecordCount();
}
UInt32	MapConnector::GetTotalRecordCount()
{
	return conector->GetTotalRecordCount();
}
