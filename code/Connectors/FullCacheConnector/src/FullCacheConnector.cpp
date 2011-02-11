
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
		notifier->Error("an interval error occured: RecordsCache count(%d) is smaller than the provided index(%d)", RecordsCache.GetCount(), index);
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

	char SqlString [MAX_SQL_QUERY_SIZE];
	sprintf_s(SqlString, MAX_SQL_QUERY_SIZE, "select * from %s", RECORDS_TABLE_NAME);
	
	RecordsCount = database->Select(SqlString);
	IntervalStart = 0;
	IntervalEnd   = RecordsCount;
	Initialized = TRUE;

	// something bad has happend
	if (RecordsCount==0) {
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

		//store data 
		GML::ML::MLRecord record;
		MEMSET((void*)&record, 0, sizeof(MLRecord));

		// count the number of features that we have
		FeaturesCount = 0;
		UInt32 vectSize = VectPtr.GetCount();

		for (UInt32 tr=0;tr<vectSize;tr++) {
			DBRecord* dbrec = VectPtr.GetPtrToObject(tr);
			if (strncmp(dbrec->Name, FEATURES_COL_PREFIX, strlen(FEATURES_COL_PREFIX))==0)
				FeaturesCount++;
		}
													
		// alloc memory for the features		
		record.Features = (double*) malloc(sizeof(double)*FeaturesCount);
		if (!record.Features ) {
			notifier->Error("could not allocated memory for currect row");
			database->FreeRow(VectPtr);
			return false;
		}
																						
		// pass through the vector to store information
		UInt32 sizeLabelColName = strlen(LABEL_COL_NAME);
		UInt32 sizeHashColname  = strlen(HASH_COL_NAME);
		UInt32 sizeFeatPrefColName = strlen(FEATURES_COL_PREFIX);

		for (UInt32 tr=0;tr<VectPtr.GetCount();tr++) {
			DBRecord * dbrec = VectPtr.GetPtrToObject(tr);

			// look for label
			if (strncmp(dbrec->Name, LABEL_COL_NAME, sizeLabelColName)==0) {

				// check if it's a signed type
				if (dbrec->Type == BOOLVAL || dbrec->Type == UINT8VAL || dbrec->Type == UINT16VAL || dbrec->Type == UINT32VAL) {
					notifier->Error("wrong data type for Label column");
					database->FreeRow(VectPtr);
					VectPtr.DeleteAll();
					return false;
				}

				record.Label = dbrec->DoubleVal;
				continue;
			}

			//look for the hash
			if (strncmp(dbrec->Name, HASH_COL_NAME, sizeHashColname)==0) {

				// check if it's indeed a Hash data type
				if (dbrec->Type != HASHVAL) {
					notifier->Error("wrong data type for Hash column");
					database->FreeRow(VectPtr);
					VectPtr.DeleteAll();
					return false;
				}

				record.Hash = dbrec->Hash;
				continue;
			}

			//look for features
			if (strncmp(dbrec->Name, FEATURES_COL_PREFIX, sizeFeatPrefColName)==0) {

				// check that features are DOUBLEVAL
				if (dbrec->Type != DOUBLEVAL) {
					notifier->Error("wrong data type for Feat column");
					database->FreeRow(VectPtr);
					VectPtr.DeleteAll();
					return false;
				}

				UInt32 nr;
				char * asciiNr = (char*) &dbrec->Name[sizeFeatPrefColName];
				nr = atoi (asciiNr);
				record.Features[nr] = dbrec->DoubleVal;
			}
		}				

		// set class data
		FeaturesCount = record.FeatCount;
		
		// put the created record in our cache
		RecordsCache.PushByRef(record);

		// free the data from database plugin
		database->FreeRow(VectPtr);
		VectPtr.DeleteAll();
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
	for (UInt32 i=0;i<RecordsCache.GetCount();i++) {
		MLRecord * rec = RecordsCache.GetPtrToObject(i);
		if (rec->Features!=NULL) {
			free(rec->Features);
		}
	}

	if (!RecordsCache.DeleteAll()) {
		notifier->Error("error deleting all values from cache");
	}
}
