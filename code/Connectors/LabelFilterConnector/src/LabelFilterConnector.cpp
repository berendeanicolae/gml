#include "LabelFilterConnector.h"

LabelFilterConnector::LabelFilterConnector()
{
	ObjectName = "LabelFilterConnector";
	
	LinkPropertyToUInt32("Operation"	,Operation		,OP_EQUAL,"!!LIST:Equal=0,Different,Bigger,Smaller,BiggerEq,SmallerEq!!");
	LinkPropertyToDouble("Value"		,Value			,1.0,"Value of the label");
}
bool LabelFilterConnector::OnInitConnectionToConnector()
{
	UInt32					tr,conCount;
	double					recLabel;
	bool					add;

	if (Indexes.Create(conector->GetRecordCount())==false)
	{
		notifier->Error("[%s] -> Unable to allocate %d indexes ",ObjectName,conector->GetRecordCount());
		return false;
	}
	// bag in list
	notifier->StartProcent("[%s] -> Filtering ",ObjectName);
	conCount = conector->GetRecordCount();
	for (tr=0;tr<conCount;tr++)
	{
		if (conector->GetRecordLabel(recLabel,tr)==false)
		{
			notifier->Error("[%s] -> Unable to read hash for record #d ",ObjectName,tr);
			return false;
		}
		switch (Operation)
		{
			case OP_EQUAL		: add = (bool)(recLabel == Value); break;
			case OP_DIFFERENT	: add = (bool)(recLabel != Value); break;
			case OP_BIGGER		: add = (bool)(recLabel >  Value); break;
			case OP_SMALLER		: add = (bool)(recLabel <  Value); break;
			case OP_BIGGER_EQ	: add = (bool)(recLabel >= Value); break;
			case OP_SMALLER_EQ	: add = (bool)(recLabel <= Value); break;
			default				: add = false; break;
		};
		if (add)
		{
			if (Indexes.Push(tr)==false)
			{
				notifier->Error("[%s] -> Unable to add record #d to index list",ObjectName,tr);
				return false;
			}
		}
		if ((tr % 1000)==0)
			notifier->SetProcent(tr,conCount);
	}
	notifier->EndProcent();
	if (Indexes.Len()==0)
	{
		notifier->Error("[%s] -> 0 records remains after filtering",ObjectName);
		return false;	
	}	
	nrRecords = Indexes.Len();
	columns.nrFeatures = conector->GetFeatureCount();
	dataMemorySize = nrRecords*sizeof(UInt32);
	return true;

}
bool LabelFilterConnector::GetRecordLabel( double &label,UInt32 index )
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}
	return conector->GetRecordLabel(label, (UInt32)Indexes.Get(index));

}
bool LabelFilterConnector::GetRecord( GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask)
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}

	return conector->GetRecord(record,(UInt32)Indexes.Get(index),recordMask);

}
bool LabelFilterConnector::GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index)
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}

	return conector->GetRecordHash(recHash,(UInt32)Indexes.Get(index));

}

bool LabelFilterConnector::CreateMlRecord( GML::ML::MLRecord &record )
{
	if (this->conector)
		return this->conector->CreateMlRecord(record);
	return false;
}
bool LabelFilterConnector::FreeMLRecord( GML::ML::MLRecord &record )
{
	if (this->conector)
		return this->conector->FreeMLRecord(record);
	return false;
}

bool LabelFilterConnector::Close()
{
	if (this->conector)
		return this->conector->Close();	 
	return false;
}
