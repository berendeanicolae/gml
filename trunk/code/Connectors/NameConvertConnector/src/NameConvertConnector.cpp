#include "NameConvertConnector.h"

NameConvertConnector::NameConvertConnector()
{
	ObjectName = "NameConvertConnector";
	
	LinkPropertyToString("FeatureNamesList",FeatureNamesList,"");
}
bool NameConvertConnector::OnInitConnectionToConnector()
{
	GML::Utils::GString		all,line;
	int						poz,count;
	
	if (all.LoadFromFile(FeatureNamesList.GetText())==false)
		return false;
	
	poz = 0;
	count = 0;
	while (all.CopyNextLine(&line,&poz))
	{
		line.Strip();
		if (line.Len()==0)
			continue;
		count++;
	}
	if (count==0)
		return false;
	NewNames = new GML::Utils::GString [count];
	poz = 0;
	count = 0;
	while (all.CopyNextLine(&line,&poz))
	{
		line.Strip();
		if (line.Len()==0)
			continue;
		NewNames[count].Set(&line);
		count++;
	}
	notifier->Info("[%s] -> Found %d names ",ObjectName,count);
	if (conector->GetFeatureCount()!=count)
		return false;
	return true;
}
bool NameConvertConnector::GetRecordLabel( double &label,UInt32 index )
{
	return conector->GetRecordLabel(label,index);
}
bool NameConvertConnector::GetRecord( GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask)
{
	return conector->GetRecord(record,index,recordMask);
}
bool NameConvertConnector::GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index)
{
	return conector->GetRecordHash(recHash,index);;
}
bool NameConvertConnector::GetFeatureName(GML::Utils::GString &str,UInt32 index)
{
	return str.Set(&NewNames[index]);
}

bool NameConvertConnector::CreateMlRecord( GML::ML::MLRecord &record )
{
	return conector->CreateMlRecord(record);
}
bool NameConvertConnector::FreeMLRecord( GML::ML::MLRecord &record )
{
	return conector->FreeMLRecord(record);
}
bool NameConvertConnector::Close()
{
	return conector->Close();
}
