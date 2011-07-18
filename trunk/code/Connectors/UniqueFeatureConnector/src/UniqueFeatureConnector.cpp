#include "UniqueFeatureConnector.h"

UniqueFeatureConnector::UniqueFeatureConnector()
{
	ObjectName = "UniqueFeatureConnector";
}
UniqueFeatureConnector::~UniqueFeatureConnector()
{
}
bool UniqueFeatureConnector::OnInitConnectionToConnector()
{
	return false;
}
bool UniqueFeatureConnector::OnInitConnectionToDataBase()
{
	return false;
}
bool UniqueFeatureConnector::Save(char *fileName)
{
	return false;
}
bool UniqueFeatureConnector::Load(char *fileName)
{
	return false;
}
bool UniqueFeatureConnector::GetRecordLabel( double &label,UInt32 index )
{
	return false;
}
bool UniqueFeatureConnector::GetRecord( GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask)
{
	return false;
}
bool UniqueFeatureConnector::GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index)
{
	return false;
}
bool UniqueFeatureConnector::GetFeatureName(GML::Utils::GString &str,UInt32 index)
{
	return false;
}

bool UniqueFeatureConnector::CreateMlRecord( GML::ML::MLRecord &record )
{
	return false;
}
bool UniqueFeatureConnector::FreeMLRecord( GML::ML::MLRecord &record )
{
	return false;
}

bool UniqueFeatureConnector::Close()
{
	return false;
}
