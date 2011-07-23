#include "RuleFilterConnector.h"

RuleFilterConnector::RuleFilterConnector()
{
	ObjectName = "RuleFilterConnector";
	
	LinkPropertyToString("RulesFile",RulesFile,"","Name of the file that contains rules");
	LinkPropertyToUInt32("Method",Method,Method_RemoveIfAllFeaturesAreSet,"!!LIST:RemoveIfAllFeaturesAreSet=0,KeepIfAllFeaturesAreSet,RemoveIfOneFeatureIsSet,KeepIfOneFeatureIsSet!!");
	
}
RuleFilterConnector::~RuleFilterConnector()
{
	Indexes.Destroy();
}
bool RuleFilterConnector::FeatureNameToIndex(GML::Utils::GString &name,UInt32 &index)
{
	GML::Utils::GString	temp;
	
	if (name.StartsWith("#"))
	{
		if (!name.Replace("#", ""))
		{
			notifier->Error("[%s] -> Failed to extract feature`s index (%s)!!!",ObjectName,name.GetText());
			return false;
		}
		if (!name.ConvertToUInt32(&index)) 			
		{
			notifier->Error("[%s] -> Failed to extract feature`s index (%s)!!!",ObjectName,name.GetText());
			return false;
		}
		if (index>=conector->GetFeatureCount())
		{
			notifier->Error("[%s] -> wrong feature`s index. Must be inside [0,%d]!!!",ObjectName, conector->GetFeatureCount());
			return false;
		}
		return true;
	}
	for (index=0;index<conector->GetFeatureCount();index++)
	{
		if (conector->GetFeatureName(temp, index) == false)
		{
			notifier->Error("[%s] -> Unable to get feature name for feature with index: %d",ObjectName, index);
			return false;
		}
		if (name.Equals(temp,true))
			return true;
	}
	notifier->Error("[%s] -> Unknown feature name: %s ",ObjectName,name.GetText());
	return false;
}
bool RuleFilterConnector::LoadFeaturesList()
{
	GML::Utils::GString		all,line;
	int						poz = 0;
	UInt32					index=0;
	
	if (all.LoadFromFile(RulesFile.GetText())==false)
	{
		notifier->Error("[%s] -> Unable to open: %s",ObjectName,RulesFile.GetText());
		return false;	
	}
	while (all.CopyNextLine(&line,&poz))
	{
		line.Strip();
		if (line.Len()==0)
			continue;
		if (FeatureNameToIndex(line,index)==false)
			return false;
		FList.Set(index,true);
	}
	if (FList.CountElements(true)==0)
	{
		notifier->Error("[%s] -> At least one feature should be present in the rule file",ObjectName,RulesFile.GetText());
		return false;	
	}
	return true;
}
bool RuleFilterConnector::Check_AllFeaturesAreSet(GML::ML::MLRecord &rec)
{
	for (UInt32 tr=0;tr<FList.Len();tr++)
	{
		if ((FList.Get(tr)) && (rec.Features[tr]==0))
			return false;
	}
	return true;
}
bool RuleFilterConnector::Check_OneFeatureIsSet(GML::ML::MLRecord &rec)
{
	for (UInt32 tr=0;tr<FList.Len();tr++)
	{
		if ((FList.Get(tr)) && (rec.Features[tr]!=0))
			return true;
	}
	return false;
}
bool RuleFilterConnector::OnInitConnectionToConnector()
{
	GML::ML::MLRecord	cRec;
	bool				keepRecord;

	if (Indexes.Create(conector->GetRecordCount())==false)
	{
		notifier->Error("[%s] -> Unable to allocate %d indexes ",ObjectName,conector->GetRecordCount());
		return false;
	}
	if (FList.Create(conector->GetFeatureCount())==false)
	{
		notifier->Error("[%s] -> Unable to create %d indexes for features",ObjectName,conector->GetFeatureCount());
		return false;
	}
	if (conector->CreateMlRecord(cRec)==false)
	{
		notifier->Error("[%s] -> Unable to crea MLRecord ",ObjectName);
		return false;
	}	
	FList.SetAll(false);
	
	if (LoadFeaturesList()==false)
		return false;
		
	notifier->StartProcent("[%s] -> Analizing records ... ",ObjectName);
	for (UInt32 tr=0;tr<conector->GetRecordCount();tr++)
	{
		if (conector->GetRecord(cRec,tr)==false)		
		{
			notifier->Error("[%s] -> Error reading #%d record from parent connector!",ObjectName,tr);
			return false;
		}
		switch (Method)
		{
			case Method_RemoveIfAllFeaturesAreSet:
				keepRecord = !Check_AllFeaturesAreSet(cRec);
				break;
			case Method_KeepIfAllFeaturesAreSet:
				keepRecord = Check_AllFeaturesAreSet(cRec);
				break;
			case Method_RemoveIfOneFeatureIsSet:
				keepRecord = !Check_OneFeatureIsSet(cRec);
				break;
			case Method_KeepIfOneFeatureIsSet:
				keepRecord = Check_OneFeatureIsSet(cRec);
				break;
			default:
				notifier->Error("[%s] -> Unknown method: %d!",ObjectName,Method);
				return false;
		};
		if (keepRecord)
		{
			if (Indexes.Push(tr)==false)
			{
				notifier->Error("[%s] -> Unable to add record #d to index list",ObjectName,tr);
				return false;
			}		
		}
		
		if ((tr % 10000)==0)
			notifier->SetProcent(tr,conector->GetRecordCount());
	}
	notifier->EndProcent();
	
	nrRecords = Indexes.Len();
	columns.nrFeatures = conector->GetFeatureCount();
	dataMemorySize = nrRecords*sizeof(UInt32);	
	return true;
}
bool RuleFilterConnector::GetRecordLabel( double &label,UInt32 index )
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}
	return conector->GetRecordLabel(label, (UInt32)Indexes.Get(index));
}
bool RuleFilterConnector::GetRecord( GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask)
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}

	return conector->GetRecord(record,(UInt32)Indexes.Get(index),recordMask);
}
bool RuleFilterConnector::GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index)
{
	if (index >= nrRecords)
	{
		notifier->Error("[%s] -> index out of range, the maximum allowed is %d",ObjectName,nrRecords-1);
		return false;
	}

	return conector->GetRecordHash(recHash,(UInt32)Indexes.Get(index));
}
bool RuleFilterConnector::CreateMlRecord( GML::ML::MLRecord &record )
{
	if (this->conector)
		return this->conector->CreateMlRecord(record);
	return false;
}
bool RuleFilterConnector::FreeMLRecord( GML::ML::MLRecord &record )
{
	if (this->conector)
		return this->conector->FreeMLRecord(record);
	return false;
}

bool RuleFilterConnector::Close()
{
	if (this->conector)
		return this->conector->Close();	 
	return false;
}