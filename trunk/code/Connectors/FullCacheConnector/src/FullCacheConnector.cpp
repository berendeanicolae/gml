
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

	if (RecordsCount<=index) {
		notifier->Error("an interval error occured: RecordsCache count(%d) is smaller than the provided index(%d)", RecordsCount, index);
		return false;
	}

	MEMCOPY((void*)&record, (void*) RecordsCache.GetPtrToObject(index), sizeof(MLRecord));
	
	return true;
}

bool FullCacheConnector::CreateMlRecord( MLRecord &record )
{	
	if (!Initialized) {
		notifier->Error("connector not initialized");
		return false;
	}	

	MEMSET((void*)&record, 0, sizeof(MLRecord));

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

	GTVector<DBRecord> VectPtr;
	MLRecord record;
	DBRecord* rec;

	UInt32	LabelPos, HashPos, vectSize;
	
	char SqlString [MAX_SQL_QUERY_SIZE];


	// connect to database
	if (!database->Connect()) 
	{
		notifier->Error("could not connect to database");
		return false;
	}

	// build the internal cache 
	
	sprintf_s(SqlString, MAX_SQL_QUERY_SIZE, "select * from %s", RECORDS_TABLE_NAME);
	
	RecordsCount = database->Select(SqlString);

	// something bad has happend
	if (RecordsCount==0) 
	{
		notifier->Error("I received 0 records from the database");
		return false;
	}
	
	IntervalStart = 0;
	IntervalEnd   = RecordsCount;
	Initialized = TRUE;	

	// fetch data
	if (!database->FetchNextRow(VectPtr)) 
	{		
		notifier->Error("error fetching record %d from database", 0);
		return false;
	}		

	// count the number of features that we have
	FeaturesCount = 0;
	vectSize = VectPtr.GetCount();

	for (UInt32 tr=0;tr<vectSize;tr++) 
	{
		rec = VectPtr.GetPtrToObject(tr);
		if (GML::Utils::GString::StartsWith(rec->Name,"Feat_",true))
			FeaturesCount++;
	}
	
	// alloc memory for the cache
	FeaturesCache = new double [FeaturesCount*RecordsCount];
	
	if (!FeaturesCache) 
	{
		notifier->Error("error allocating memory for the internal cache");
		return false;
	}

	record.Features = (double*)&FeaturesCache[0*FeaturesCount];

	for (UInt32 tr=0;tr<VectPtr.GetCount();tr++) 
	{
		DBRecord * dbrec = VectPtr.GetPtrToObject(tr);

		// look for label
		if (GML::Utils::GString::Equals(dbrec->Name, "Label", true)) 
		{
			// check if it's a signed type
			if (dbrec->Type == BOOLVAL || dbrec->Type == UINT8VAL || dbrec->Type == UINT16VAL || dbrec->Type == UINT32VAL) {
				notifier->Error("wrong data type for Label column");
				database->FreeRow(VectPtr);
				VectPtr.DeleteAll();
				return false;
			}

			record.Label = dbrec->DoubleVal;
			LabelPos = tr;
			continue;
		}

		//look for the hash
		if (GML::Utils::GString::Equals(dbrec->Name, "Hash", true)) 
		{
			// check if it's indeed a Hash data type
			if (dbrec->Type != HASHVAL) {
				notifier->Error("wrong data type for Hash column");
				database->FreeRow(VectPtr);
				VectPtr.DeleteAll();
				return false;
			}

			record.Hash = dbrec->Hash;
			HashPos = tr;
			continue;
		}

		//look for features
		if (GML::Utils::GString::StartsWith(dbrec->Name, "Feat_", true)) 
		{
			// check that features are DOUBLEVAL
			if (dbrec->Type != DOUBLEVAL && dbrec->Type != FLOATVAL) {
				notifier->Error("wrong data type for Feat column");
				database->FreeRow(VectPtr);
				VectPtr.DeleteAll();
				return false;
			}

			UInt32 nr;
			if (GML::Utils::GString::ConvertToUInt32(&dbrec->Name[5],&nr)==false)
			{
				notifier->Error("Invalid number for Feat_xxx column: %s",dbrec->Name);
				database->FreeRow(VectPtr);
				VectPtr.DeleteAll();
				return false;
			}
			record.Features[nr] = dbrec->DoubleVal;
		}
	}

	// put the created record in our cache
	RecordsCache.PushByRef(record);

	// free the data from database plugin
	database->FreeRow(VectPtr);

	// prepare the vector for our next round
	VectPtr.DeleteAll();
	
	for (UInt32 i=1;i<RecordsCount;i++) 
	{
		// fetch data
		if (!database->FetchNextRow(VectPtr)) 
		{
			notifier->Error("error fetching record %d from database", 0);
			return false;
		}	
													
		// put pointer from cache		
		record.Features = (double*)&FeaturesCache[i*FeaturesCount];	
		record.FeatCount = FeaturesCount;
		record.Label = VectPtr[LabelPos].DoubleVal;
		record.Hash =  VectPtr[HashPos].Hash;
						
		// check to see if we have fields above the HashPos
		if (VectPtr.GetCount()<=HashPos) 
		{
			notifier->Error("error - the database vector has no features");
			return false;
		}

		// pass through the vector to store information		
		for (UInt32 tr=LabelPos+1;tr<VectPtr.GetCount();tr++) 
		{
			DBRecord * dbrec = VectPtr.GetPtrToObject(tr);

			// check that features are DOUBLEVAL
			if (dbrec->Type != DOUBLEVAL && dbrec->Type != FLOATVAL) 
			{
				notifier->Error("wrong data type for Feat column");
				database->FreeRow(VectPtr);
				VectPtr.DeleteAll();
				return false;
			}

			UInt32 nr;
			if (GML::Utils::GString::ConvertToUInt32(&dbrec->Name[5],&nr)==false)
			{
				notifier->Error("Invalid number for Feat_xxx column: %s",dbrec->Name);
				database->FreeRow(VectPtr);
				VectPtr.DeleteAll();
				return false;
			}
			record.Features[nr] = dbrec->DoubleVal;
		}				
		
		// put the created record in our cache
		RecordsCache.PushByRef(record);

		// free the data from database plugin
		database->FreeRow(VectPtr);

		// prepare the vector for our next round
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

	 FeaturesCache = NULL;
	 RecordsCache.DeleteAll();
}

FullCacheConnector::~FullCacheConnector()
{
	if (FeaturesCache != NULL) 	
		free(FeaturesCache);
	
	if (!RecordsCache.DeleteAll()) 
		notifier->Error("error deleting all values from cache");	
}
