#pragma once

#include "Compat.h"
#include "INotifier.h"
#include "MLRecord.h"
#include "IDataBase.h"


namespace GML
{
	namespace ML
	{
		enum RecordMask
		{
			RECORD_STORE_HASH = 1,
		};
		struct TableColumnIndexes
		{
			UInt32		nrFeatures;
			Int32		indexLabel;
			Int32		indexHash;
			Int32		*indexFeature;
		};
		class EXPORT IConnector : public GML::Utils::GMLObject
		{
		private:
			bool						Init(GML::Utils::INotifier &Notifier,GML::DB::IDataBase *Database,GML::ML::IConnector *connecor,char *attributeString);
		protected:
			GML::Utils::INotifier		*notifier;			
			GML::DB::IDataBase			*database;
			GML::ML::IConnector			*conector;
			GML::Utils::GString			DataFileName;			
			GML::Utils::GString			Query;
			GML::Utils::GString			CountQuery;
			TableColumnIndexes			columns;
			UInt32						CachedRecords;
			bool						StoreRecordHash;
			
			void						ClearColumnIndexes();
			bool						UpdateDoubleValue(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr,Int32 index,double &value);
			bool						UpdateHashValue(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr,Int32 index,GML::DB::RecordHash &recHash);
			bool						UpdateColumnInformations(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr);
			bool						QueryRecordsCount(char *CountQueryStatement,UInt32 &recordsCount);
			bool						UpdateColumnInformations(char *QueryStatement);


			virtual bool				OnInit();
			virtual bool				OnInitConnectionToDataBase();
			virtual bool				OnInitConnectionToConnector();
			virtual bool				OnInitConnectionToCache();
			
		public:	
			IConnector();

			virtual bool				Init(GML::Utils::INotifier &Notifier,GML::DB::IDataBase &Database,char *attributeString=NULL);
			virtual bool				Init(GML::ML::IConnector &conector,char *attributeString=NULL);
			virtual bool				Init(GML::Utils::INotifier &Notifier,char *attributeString=NULL);
			virtual bool				Save(char *fileName);
			virtual bool				Load(char *fileName);


			virtual bool Close()=0;
	
			/*
			 *Usage: set the interval for this paralel unit's database
			 *Params:
			 *	-  UInt32 start: the start unit
			 *			!!! this parameter is 0 indexed (not 1)
			 *	-  UInt32 end: the end unit (this unit will not be included in the interval)	 
			 */
			virtual bool SetRecordInterval(UInt32 start, UInt32 end)=0;

			/*
			 * Usage: allocated, create and return a MLRecord structure instance
			 * Return: the allocated MlRecord structure or NULL if out of memory
			 */
			virtual bool CreateMlRecord (MLRecord &record)=0;

			/*
			 *Usage: Get a single record of data
			 *Params:
			 *	- OUTPUT MLRecord &record: the record to be fetched
			 *	- INPUT  UInt32 index: the record index
			 */
			virtual bool GetRecord(MLRecord &record,UInt32 index,UInt32 recordMask=0)=0;

			virtual bool GetRecordLabel(double &label,UInt32 index)=0;

			virtual bool GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index);

			/*
			 * Usage: Free a MLRecord structure
			 * Params:
			 *	- INPUT MLRecord *record: a pointer to a structure received through a GetRecord call
			 */
			virtual bool FreeMLRecord(MLRecord &record)=0;


			/*
			 * Usage: Get the number of features 
			 */
			virtual UInt32 GetFeatureCount()=0;

			/*
			 * Usage:	- Get the number of records in the database	for the current paralel unit
			 *			- if no interval has been specified it returns the total number of records
			 */
			virtual UInt32 GetRecordCount()=0;
	
			/*
			 * Usage Get the total number of records in the database
			 */
			virtual UInt32 GetTotalRecordCount()=0;
		};

	}
}


