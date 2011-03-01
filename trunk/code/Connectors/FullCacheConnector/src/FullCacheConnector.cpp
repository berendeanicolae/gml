
#include "FullCacheConnector.h"

UInt32 FullCacheConnector::GetRecordCount()
{
	if (!Initialized) {
		notifier->Error("connector not initialized");
		return 0;
	}

	return RecordCount;
}

UInt32 FullCacheConnector::GetFeatureCount()
{
	if (!Initialized) {
		notifier->Error("connector not initialized");
		return 0;
	}

	return FeatureCount;		
}

bool FullCacheConnector::GetRecord( MLRecord &record,UInt32 index )
{
	if (index < IntervalStart || index >= IntervalEnd) {
		notifier->Error("index is out of range, it has to be in interval (%d - %d)", IntervalStart, IntervalEnd);
		return false;
	}

	if (RecordCount<=index) {
		notifier->Error("an interval error occured: RecordsCache count(%d) is smaller than the provided index(%d)", RecordCount, index);
		return false;
	}

	record.Features = (double*) &FeatureCache[index*FeatureCount]; 
	record.FeatCount = FeatureCount;
	record.Label = LabelStorage[index];	
	
	return true;
}

bool FullCacheConnector::CreateMlRecord( MLRecord &record )
{	
	if (!Initialized) {
		notifier->Error("connector not initialized");
		return false;
	}	

	record.FeatCount = FeatureCount;

	return true;
}

bool FullCacheConnector::SetRecordInterval( UInt32 start, UInt32 end )
{
	if (start >= RecordCount || end >= RecordCount) {
		notifier->Error("could not set interval margin above value %d", RecordCount);
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
	GTFVector<DBRecord> VectPtr;
	DBRecord* rec;

	UInt32	LabelPos, vectSize;
	double  * FeaturesPtr;

	notifier->Info("FullCacheConnector loading data");	
	
	char SqlString [MAX_SQL_QUERY_SIZE];

	if (VectPtr.Create(1024)==false)
	{
		notifier->Error("Unable to preallocate memory for records !");
		return false;
	}

	// connect to database
	if (!database->Connect()) 
	{
		notifier->Error("could not connect to database");
		return false;
	}

	// build the internal cache 
	
	if (SelectQuery.Equals("*"))
		sprintf_s(SqlString, MAX_SQL_QUERY_SIZE, "select * from %s", TableName);
	else
		sprintf_s(SqlString, MAX_SQL_QUERY_SIZE, "%s", SelectQuery.GetText());
	
	RecordCount = database->Select(SqlString);

	// something bad has happend
	if (RecordCount==0) 
	{
		notifier->Error("I received 0 records from the database");
		return false;
	}
	
	IntervalStart = 0;
	IntervalEnd   = RecordCount;
	Initialized = TRUE;	

	// fetch data
	if (!database->FetchNextRow(VectPtr)) 
	{		
		notifier->Error("error fetching record %d from database", 0);
		return false;
	}		

	// count the number of features that we have
	FeatureCount = 0;
	vectSize = VectPtr.GetCount();

	for (UInt32 tr=0;tr<vectSize;tr++) 
	{
		rec = VectPtr.GetPtrToObject(tr);
		if (GML::Utils::GString::StartsWith(rec->Name,"Feat_",true))
			FeatureCount++;
	}
	
	// alloc memory for the cache
	FeatureCache = new double [FeatureCount*RecordCount];
	LabelStorage  = new double [RecordCount];
	
	if (!FeatureCache || !LabelStorage) 
	{
		notifier->Error("error allocating memory for the internal cache");
		return false;
	}

	FeaturesPtr = (double*)&FeatureCache[0*FeatureCount];	

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

			LabelStorage[0] = dbrec->DoubleVal;
			LabelPos = tr;
			continue;
		}

		/*
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
		*/

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
			FeaturesPtr[nr] = dbrec->DoubleVal;
		}
	}
	
	// free the data from database plugin
	database->FreeRow(VectPtr);

	// prepare the vector for our next round
	VectPtr.DeleteAll();
	
	for (UInt32 i=1;i<RecordCount;i++) 
	{		
		// fetch data
		if (!database->FetchNextRow(VectPtr)) 
		{
			notifier->Error("error fetching record %d from database", 0);
			return false;
		}	
													
		// put pointer from cache		
		FeaturesPtr = (double*)&FeatureCache[i*FeatureCount];	
		
		LabelStorage[i] = VectPtr[LabelPos].DoubleVal;		
						
		// check to see if we have fields above the HashPos
		if (VectPtr.GetCount()<=LabelPos+1) 
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
			FeaturesPtr[nr] = dbrec->DoubleVal;
		}				
				
		// free the data from database plugin
		database->FreeRow(VectPtr);

		// prepare the vector for our next round
		VectPtr.DeleteAll();
	}	

	notifier->Info("FullCacheConnector data (Records=%d,Features=%d)",RecordCount,FeatureCount);

	return true;
}

bool FullCacheConnector::FreeMLRecord( MLRecord &record )
{	
	return true;
}

bool FullCacheConnector::Close()
{
	return database->Disconnect();	
}

UInt32 FullCacheConnector::GetTotalRecordCount()
{
	if (!Initialized) return 0;
	return RecordCount;
}

FullCacheConnector::FullCacheConnector()
{
	 RecordCount = 0;
	 FeatureCount = 0;
	 Initialized = FALSE;

	 FeatureCache = NULL;
	 LabelStorage  = NULL;	 	 	 
}

FullCacheConnector::~FullCacheConnector()
{
	if (FeatureCache != NULL) 	
		delete FeatureCache;

	if (LabelStorage != NULL)
		delete LabelStorage;
}

bool FullCacheConnector::GetRecordLabel( double &label,UInt32 index )
{
	if (index < IntervalStart || index >= IntervalEnd)
		return false;

	label = LabelStorage[index];
	return true;
}
