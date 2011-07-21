#include "MapConnector.h"

MapConnector::MapConnector()
{
	ObjectName = "MapConnector";

	LinkPropertyToUInt32("MapMethod",mapMethod,UseAND,	"!!LIST:UseAND=0,UseOR,UseXOR,UseAnd+Or,UseMultiply,UseAddition,UseFeatAverage,Negate,Interval!!\n"
														"Selects the method to be used for mapping\n"
														"* UseAND -> feat(1..n) will become feat`(1..m) where\n"														
														"\n"
														);
	LinkPropertyToDouble("StartInterval",startMapInterval,0.0,"Value for the start of the interval");
	LinkPropertyToDouble("EndInterval",endMapInterval,1.0,"Value for the end of the interval");
}

bool	MapConnector::CreateNameIndexes()
{
	UInt32	tr,gr;
	if (NameIndex.Create(columns.nrFeatures)==false)
	{
		notifier->Error("[%s] -> Unable to alloc %d for name indexes",ObjectName,columns.nrFeatures);
		return false;
	}
	switch (mapMethod)
	{
		case UseAND:
		case UseXOR:
		case UseOR:
		case UseMultiply:
		case UseAddition:
			for (tr=0;tr<conector->GetFeatureCount();tr++)
			{
				for (gr=tr;gr<conector->GetFeatureCount();gr++)
				{
					if (NameIndex.Push((tr<<16) | (gr & 0xFFFF))==false)
					{
						notifier->Error("[%s] -> Unable to add map (%d,%d) to list",ObjectName,tr,gr);
						return false;
					}
				}
			}
			break;
		case Negate:
		case Interval:
			for (tr=0;tr<conector->GetFeatureCount();tr++)
			{
				if (NameIndex.Push(tr)==false)
				{
					notifier->Error("[%s] -> Unable to add map (%d) to list",ObjectName,tr);
					return false;
				}
			}
			break;
		default:
			notifier->Error("[%s] -> Method %d doesn`t support name indexes !",ObjectName,mapMethod);
			return false;
	};
	return true;
}
bool	MapConnector::OnInitConnectionToConnector()
{
	columns.nrFeatures = 0;
	// calculez cate feature-uri o sa am in functie de metoda

	switch (mapMethod)
	{
		case UseAND:
		case UseXOR:
		case UseOR:
		case UseMultiply:
		case UseAddition:
			columns.nrFeatures = (conector->GetFeatureCount() * (conector->GetFeatureCount()+1))>>1;
			break;
		case UseAnd_Or:
			columns.nrFeatures = (conector->GetFeatureCount() * (conector->GetFeatureCount()+1));
			break;
		case UseFeatAverage:
			columns.nrFeatures = conector->GetFeatureCount()+1;
			break;
		case Negate:
		case Interval:
			columns.nrFeatures = conector->GetFeatureCount();
			break;
		default:
			notifier->Error("[%s] -> Unknown method ID = %d",ObjectName,mapMethod);
			return false;
	};
	if (StoreFeaturesName)
	{
		if (CreateNameIndexes()==false)
		{
			notifier->Error("[%s] -> Unable to create feature name indexes",ObjectName);
			return false;
		}
	}

	notifier->Info("[%s] -> Mapping %d features to %d features ",ObjectName,conector->GetFeatureCount(),columns.nrFeatures);
	nrRecords = conector->GetRecordCount();
	
	return true;
}
bool	MapConnector::Close()
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
	record.FeatCount = columns.nrFeatures;
	return ((record.Features = new double[columns.nrFeatures])!=NULL);
}
bool	MapConnector::GetRecord(GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask)
{
	UInt32		tr,gr,pCount;
	double		*p1,*p2,*pMap;
	double		sum;

	if (conector->GetRecord(*record.Parent,index,recordMask)==false)
		return false;
	pCount = conector->GetFeatureCount();
	record.Weight = record.Parent->Weight;
	record.Label = record.Parent->Label;
	record.FeatCount = columns.nrFeatures;
	if (recordMask & GML::ML::ConnectorFlags::STORE_HASH)
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
		case UseFeatAverage:
			p1 = record.Parent->Features;
			sum = 0.0;
			for (tr=0;tr<pCount;tr++,p1++,pMap++)
			{
				(*pMap) = (*p1);
				sum+=(*p1);
			}
			(*pMap) = sum/((double)pCount);
			break;
		case Negate:
			p1 = record.Parent->Features;
			for (tr=0;tr<pCount;tr++,p1++,pMap++)
			{
				(*pMap) = 1.0-(*p1);
			}
			break;
		case Interval:
			p1 = record.Parent->Features;
			sum = endMapInterval-startMapInterval;
			for (tr=0;tr<pCount;tr++,p1++,pMap++)
			{
				(*pMap) = (*p1) * sum + startMapInterval;
			}
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
bool	MapConnector::GetFeatureName(GML::Utils::GString &str,UInt32 index)
{
	GML::Utils::GString	s1,s2;
	UInt32				value,v1,v2;

	if (StoreFeaturesName==false)
	{
		notifier->Error("[%s] -> Feature names are only supported when 'StoreFeaturesName' is set",ObjectName);
		return false;
	}
	if (index>=columns.nrFeatures)
		return false;

	value = NameIndex[index];
	v1 = value >> 16;
	v2 = value & 0xFFFF;

	switch (mapMethod)
	{
		case UseAND:
			if ((conector->GetFeatureName(s1,v1)==false) || (conector->GetFeatureName(s1,v2)==false))
				return false;
			return str.SetFormated("%s AND %s",s1.GetText(),s2.GetText());
		case UseOR:
			if ((conector->GetFeatureName(s1,v1)==false) || (conector->GetFeatureName(s1,v2)==false))
				return false;
			return str.SetFormated("%s OR %s",s1.GetText(),s2.GetText());
		case UseXOR:
			if ((conector->GetFeatureName(s1,v1)==false) || (conector->GetFeatureName(s1,v2)==false))
				return false;
			return str.SetFormated("%s XOR %s",s1.GetText(),s2.GetText());
		case UseMultiply:
			if ((conector->GetFeatureName(s1,v1)==false) || (conector->GetFeatureName(s1,v2)==false))
				return false;
			return str.SetFormated("%s MUL %s",s1.GetText(),s2.GetText());
		case UseAddition:
			if ((conector->GetFeatureName(s1,v1)==false) || (conector->GetFeatureName(s1,v2)==false))
				return false;
			return str.SetFormated("%s ADD %s",s1.GetText(),s2.GetText());
		case Negate:
		case Interval:
			return conector->GetFeatureName(str,index);
	}
	notifier->Error("[%s] -> Unable to extract feature name for index : %d ",ObjectName,index);
	return false;
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

