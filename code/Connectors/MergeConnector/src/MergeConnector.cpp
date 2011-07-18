#include "MergeConnector.h"

MergeConnector::MergeConnector()
{
	ObjectName = "MergeConnector";
	Translate = NULL;
}
MergeConnector::~MergeConnector()
{
	Close();
}
GML::ML::IConnector* MergeConnector::IndexToConnector(UInt32 &index)
{
	GML::Utils::Interval	*t;
	if (Translate==NULL)
		return NULL;
	t = Translate;
	for (UInt32 tr=0;tr<connectorsCount;tr++,t++)
	{
		if ((index>=t->Start) && (index<t->End))
		{
			index-=t->Start;
			return connectors[tr];
		}		
	}
	return NULL;
}
bool MergeConnector::OnInitConnectionToConnector()
{
	UInt32					tr,gr;
	GML::Utils::GString		featName1,featName2;
	
	// verific sa fie toti aceeasi conectori
	if ((Translate = new GML::Utils::Interval[connectorsCount])==NULL)
	{
		notifier->Error("[%s] -> Unable to allocate memory for Translate vector",ObjectName);
		return false;	
	}
	Translate[0].Set(0,connectors[0]->GetRecordCount());
	notifier->StartProcent("[%s] -> Testing compatibility ... ",ObjectName);
	for (tr=1;tr<connectorsCount;tr++)
	{
		if (GML::Utils::GString::Equals(connectors[0]->GetObjectName(),connectors[tr]->GetObjectName())==false)
		{
			notifier->Error("[%s] -> All connectors in a merge connector should be of the same type",ObjectName);
			return false;
		}
		if (connectors[tr]->GetFeatureCount()!=connectors[0]->GetFeatureCount())
		{
			notifier->Error("[%s] -> All connectors in a merge connector should have the same number of features !",ObjectName);
			return false;	
		}
		for (gr=0;gr<connectors[0]->GetFeatureCount();gr++)
		{
			if (connectors[0]->GetFeatureName(featName1,gr)==false)
			{
				notifier->Error("[%s] -> Unable to read feature #%d from connector 0!",ObjectName,gr);
				return false;			
			}
			if (connectors[tr]->GetFeatureName(featName2,gr)==false)
			{
				notifier->Error("[%s] -> Unable to read feature #%d from connector %d!",ObjectName,gr,tr);
				return false;			
			}
			if (featName1.Equals(&featName2)==false)
			{
				notifier->Error("[%s] -> Features name don`t match (%s != %s)",ObjectName,featName1.GetText(),featName2.GetText());
				return false;			
			}
		}
		Translate[tr].Set(Translate[tr-1].Start,Translate[tr-1].Start+connectors[tr]->GetRecordCount());
		notifier->SetProcent(tr,connectorsCount);
	}
	notifier->EndProcent();
	// sunt compatibile - creez tabele cu linkuri
	nrRecords = Translate[connectorsCount].End;
	columns.nrFeatures = conector->GetFeatureCount();
	dataMemorySize = connectorsCount*sizeof(GML::Utils::Interval);	
	return true;
}
bool MergeConnector::GetRecordLabel( double &label,UInt32 index )
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}
	GML::ML::IConnector* c = IndexToConnector(index);
	if (c == NULL)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;	
	}
	return c->GetRecordLabel(label, index);
}
bool MergeConnector::GetRecord( GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask)
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}
	GML::ML::IConnector* c = IndexToConnector(index);
	if (c == NULL)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;	
	}
	return c->GetRecord(record,index,recordMask);
}
bool MergeConnector::GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index)
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}
	GML::ML::IConnector* c = IndexToConnector(index);
	if (c == NULL)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;	
	}
	return c->GetRecordHash(recHash,index);
}


bool MergeConnector::CreateMlRecord( GML::ML::MLRecord &record )
{
	if (this->conector)
		return this->conector->CreateMlRecord(record);
	return false;
}
bool MergeConnector::FreeMLRecord( GML::ML::MLRecord &record )
{
	if (this->conector)
		return this->conector->FreeMLRecord(record);
	return false;
}
bool MergeConnector::Close()
{
	if (Translate!=NULL)
		delete Translate;
	Translate = NULL;
	return true;
}
bool MergeConnector::AllowConnectors(UInt32 count)
{
	return (count>1);
}