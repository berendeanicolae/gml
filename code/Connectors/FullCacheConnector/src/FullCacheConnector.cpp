
#include "FullCacheConnector.h"

UInt32 FullCacheConnector::GetRecordCount()
{
	if (!Initialized) {
		notifier->Error("connector not initialized");
		return false;
	}

	return RecordsCount;
}

UInt32 FullCacheConnector::GetFeatureCount()
{
	if (!Initialized) {
		notifier->Error("connector not initialized");
		return false;
	}

	return FeaturesCount;		
}

bool FullCacheConnector::GetRecord( MLRecord &record,UInt32 index )
{
	if (index < IntervalStart || index >= IntervalEnd) {
		notifier->Error("index is out of range, it has to be in interval (%d - %d)", IntervalStart, IntervalEnd);
		return false;
	}

	if (RecordsCache.GetCount()<=index) {
		notifier->Error("an interval error occured: RecordsCache count(%d) is smaller than provided index(%d)", RecordsCache.GetCount(), index);
		return false;
	}

	MLRecord * rec = RecordsCache.GetPtrToObject(index);

	record.Hash = rec->Hash;
	record.FeatCount = rec->FeatCount;
	record.Label = rec->Label;
	MEMCOPY(record.Features, rec->Features, sizeof(double)*rec->FeatCount);	

	return true;
}

bool FullCacheConnector::CreateMlRecord( MLRecord &record )
{	
	if (!Initialized) {
		notifier->Error("connector not initialized");
		return false;
	}

	record.Features = (double*) malloc(sizeof(double)*FeaturesCount);
	if (!record.Features ) {											 
		notifier->Error("could not allocated memory for creating a MLRecord");		
		return false;
	}

	record.FeatCount = FeaturesCount;
	record.Hash = *((RecordHash*)NULL);
	record.Label = 0;
	record.Weight = 0;

	return true;
}

bool FullCacheConnector::SetRecordInterval( UInt32 start, UInt32 end )
{
	if (start >= RecordsCount || end >= RecordsCount) {
		notifier->Error("could not set interval margin above value %d", RecordsCount);
		return false;
	}

	if (end < start) {
		notifier->Error("interval end margin could not be smaller than start margin");
		return false;
	}

	IntervalStart = start;
	IntervalEnd   = end;

	return true;
}

bool FullCacheConnector::OnInit()
{
	// connect to database
	if (!database->Connect()) {
		notifier->Error("could not connect to database");
		return false;
	}

	// build the internal cache 
	
	RecordsCount = database->Select();
	IntervalStart = 0;
	IntervalEnd   = RecordsCount;
	Initialized = TRUE;

	// something bad has happend
	if (RecordsCount==0) 
	{
		notifier->Error("I received 0 records from the database");
		return false;
	}

	for (UInt32 i=0;i<RecordsCount;i++) {
		GML::Utils::GTVector<GML::DB::DBRecord> VectPtr;
		
		// fetch data
		if (!database->FetchNextRow(VectPtr)) {
			notifier->Error("error fetching record %d from database", i);
			return false;
		}

		//check data count
		if (VectPtr.GetCount()!=4) {
			notifier->Error("incorrect table format, the received vector does not have 4 fields");
			return false;
		}

		// check data types
		if (VectPtr[0].Type != UINT32VAL || 
			VectPtr[1].Type != ASCIISTTVAL ||
			VectPtr[2].Type != BOOLVAL ||
			VectPtr[3].Type != BYTESVAL) {
				notifier->Error("incorrect data types");
				database->FreeRow(VectPtr);
				return false;
		}

		//store data 
		GML::ML::MLRecord record;

		MEMCOPY((UInt8*)&record.Hash, VectPtr[1].BytesVal, 16);
		record.Label = VectPtr[2].BoolVal;
		record.FeatCount = VectPtr[3].Size;

		// set class data
		FeaturesCount = record.FeatCount;
		
		record.Features = (double*) malloc(sizeof(double)*FeaturesCount);
		if (!record.Features ) {
			notifier->Error("could not allocated memory for currect row");
			database->FreeRow(VectPtr);
			return false;
		}

		// copy the actual features
		for (UInt32 k=0;k<FeaturesCount;k++) {
			record.Features[k]  = (double) VectPtr[4].BytesVal[k];
		}		

		// put the created record in our cache
		RecordsCache.PushByRef(record);

		// free the data from database plugin
		database->FreeRow(VectPtr);

	}	

	return true;
}

bool FullCacheConnector::FreeMLRecord( MLRecord &record )
{
	if (record.Features!=NULL) 
		free(record.Features);
	return true;
}

bool FullCacheConnector::Close()
{
	database->Disconnect();
	return true;
}

UInt32 FullCacheConnector::GetTotalRecordCount()
{
	if (!Initialized) return 0;

	return RecordsCount;
}

FullCacheConnector::FullCacheConnector()
{
	 RecordsCount = 0;
	 FeaturesCount = 0;
	 Initialized = FALSE;
}

FullCacheConnector::~FullCacheConnector()
{
	if (!RecordsCache.DeleteAll()) {
		notifier->Error("error deleting all values from cache");
	}
}
