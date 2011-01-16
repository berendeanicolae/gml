
#include "XorMLDatabase.h"


UInt32 XorMlDatabase::GetRecordCount()
{
	return 2;
}

UInt32 XorMlDatabase::GetFeatureCount()
{
	return 2;
}

bool XorMlDatabase::Init( INotifier &Notifier,IDatabase &Database )
{
	this->notifier = &Notifier;
	this->database = &Database;
	
	this->database->Connect();
	this->database->Select();

	return true;
}

bool XorMlDatabase::GetRecord( MLRecord &record,UInt32 index )
{
	DbRecordVect vect;
	if (database->FetchRowNr(vect, index)== false) 
	{
		notifier->Notify("error fetching data\n");
		return false;
	}

	record.FeatCount = vect.GetCount();
	record.Features[0] = (double)vect[0].UInt32Val;
	record.Features[1] = (double)vect[1].UInt32Val;
	record.Label = (index==0)?0:1;

	return true;
}

bool XorMlDatabase::SetRecordInterval( UInt32 start, UInt32 end )
{
	return true;
}

bool XorMlDatabase::FreeMLRecord( MLRecord &record )
{
	if (record.Features)
		free(record.Features);
	return true;
}

bool XorMlDatabase::Close()
{
	return true;
}

UInt32 XorMlDatabase::GetTotalRecordCount()
{
	return 2;
}

XorMlDatabase::XorMlDatabase()
{
	
}

XorMlDatabase::~XorMlDatabase()
{

}

bool XorMlDatabase::CreateMlRecord (MLRecord& record)
{	
	double * feat = (double*) malloc(2*sizeof(double));
	if (feat == NULL) 
	{
		return NULL;;
	}

	record.Features = feat;
	record.FeatCount = 2;
	
	record.Hash.Value[0] = 0;
	record.Hash.Value[1] = 0;
	record.Hash.Value[2] = 0;
	record.Hash.Value[3] = 0;

	record.Weight = 1;	

	return true;
}
