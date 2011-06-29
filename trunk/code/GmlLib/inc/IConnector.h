#pragma once

#include "Compat.h"
#include "INotifier.h"
#include "MLRecord.h"
#include "IDataBase.h"
#include "File.h"


namespace GML
{
	namespace ML
	{
		namespace ConnectorFlags
		{
			enum
			{
				STORE_HASH = 1,
				STORE_FEATURE_NAME = 2,
			};
		};
		struct TableColumnIndexes
		{
			UInt32				nrFeatures;
			Int32				indexLabel;
			Int32				indexHash;
			Int32				*indexFeature;		
		};
		struct CacheHeader
		{
			char				MagicName[32];
			UInt32				nrRecords;
			UInt32				nrFeatures;
			UInt32				StoreFlags;
			UInt32				Flags;

		};
		class EXPORT IConnector : public GML::Utils::GMLObject
		{
		private:
			bool										Init(GML::Utils::INotifier &Notifier,GML::DB::IDataBase *Database,GML::ML::IConnector *connecor,char *attributeString);
		protected:
			GML::Utils::INotifier						*notifier;			
			GML::DB::IDataBase							*database;
			GML::ML::IConnector							*conector;
			GML::Utils::GString							DataFileName;			
			GML::Utils::GString							Query;
			GML::Utils::GString							CountQuery;
			TableColumnIndexes							columns;
			UInt32										CachedRecords;
			UInt32										nrRecords,nrFeatures;
			bool										StoreRecordHash;
			bool										StoreFeaturesName;
			GML::Utils::GTFVector<GML::DB::RecordHash>	Hashes;
			GML::Utils::GTFVector<UInt32>				indexFeatureNames;
			GML::Utils::GTFVector<UInt8>				dataFeaturesNames;
			GML::Utils::File							file;

			
			void						ClearColumnIndexes();
			bool						UpdateDoubleValue(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr,Int32 index,double &value);
			bool						UpdateHashValue(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr,Int32 index,GML::DB::RecordHash &recHash);
			bool						UpdateColumnInformations(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr);
			bool						QueryRecordsCount(char *CountQueryStatement,UInt32 &recordsCount);
			bool						UpdateColumnInformations(char *QueryStatement);
			bool						AddColumnName(char *name);
			
			bool						CreateCacheFile(char *fileName,char *sigName,CacheHeader *header,UInt32 headerSize,UInt32 extraFlags=0);
			bool						OpeanCacheFile(char *fileName,char *sigName,CacheHeader *header,UInt32 headerSize);
			void						CloseCacheFile();
			bool						SaveRecordHashes();
			bool						LoadRecordHashes();


			void						AddDataBaseProperties();
			void						AddCacheProperties();

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


			virtual bool				Close()=0;	
			virtual bool				CreateMlRecord (MLRecord &record)=0;
			virtual bool				FreeMLRecord(MLRecord &record)=0;

			virtual bool				GetRecord(MLRecord &record,UInt32 index,UInt32 recordMask=0)=0;
			virtual bool				GetRecordLabel(double &label,UInt32 index)=0;
			virtual bool				GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index);
			virtual bool				GetFeatureName(GML::Utils::GString &str,UInt32 index);

			virtual UInt32				GetFeatureCount();
			virtual UInt32				GetRecordCount();	
			virtual UInt32				GetTotalRecordCount()=0;

			virtual bool				SetRecordInterval(UInt32 start, UInt32 end)=0;
		};

	}
}


