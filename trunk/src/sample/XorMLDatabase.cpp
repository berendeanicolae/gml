
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
	if (&record)
		free(&record);

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

MLRecord* XorMlDatabase::CreateMlRecord ()
{
	MLRecord * mlr = (MLRecord*) malloc(sizeof(MLRecord));
	if (mlr == NULL) return NULL;

	double * feat = (double*) malloc(2*sizeof(double));
	if (feat == NULL) 
	{
		free(mlr);
		return NULL;;
	}

	mlr->Features = feat;
	mlr->FeatCount = 2;
	
	mlr->Hash.Value[0] = 0;
	mlr->Hash.Value[1] = 0;
	mlr->Hash.Value[2] = 0;
	mlr->Hash.Value[3] = 0;

	mlr->Weight = 1;	

	return mlr;
}
