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
		protected:
			GML::Utils::INotifier						*notifier;
		public:			
			GML::DB::IDataBase							*database;
			GML::ML::IConnector							*conector;
			GML::ML::IConnector							**connectors;
			UInt32										connectorsCount;
		protected:
			GML::Utils::GString							DataFileName;			
			GML::Utils::GString							Query;
			GML::Utils::GString							CountQuery;
			TableColumnIndexes							columns;
			UInt32										CachedRecords;
			UInt32										nrRecords;
			UInt64										dataMemorySize;
			bool										StoreRecordHash;
			bool										StoreFeaturesName;
			GML::Utils::GTFVector<GML::DB::RecordHash>	Hashes;
			GML::Utils::GTFVector<UInt32>				indexFeatureNames;
			GML::Utils::GTFVector<UInt8>				dataFeaturesNames;
			GML::Utils::File							file;

			
			void						ClearColumnIndexes();
			bool						UpdateDoubleValue(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr,Int32 index,double &value);
			bool						UpdateHashValue(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr,Int32 index,GML::DB::RecordHash &recHash);
			bool						UpdateColumnInformationsFromDataBase();
			bool						UpdateFeaturesNameFromConnector();
			bool						AddColumnName(char *name);
			
			bool						CreateCacheFile(char *fileName,char *sigName,CacheHeader *header,UInt32 headerSize,UInt32 extraFlags=0);
			bool						OpeanCacheFile(char *fileName,char *sigName,CacheHeader *header,UInt32 headerSize);
			void						CloseCacheFile();
			bool						SkipRecordHashes();
			bool						SaveRecordHashes();
			bool						LoadRecordHashes();
			bool						SaveFeatureNames();
			bool						LoadFeatureNames();
			bool						SaveRecordHashesAndFeatureNames();
			bool						LoadRecordHashesAndFeatureNames(CacheHeader *h);


			void						AddDataBaseProperties();
			void						AddCacheProperties();
			void						AddStoreProperties();

			virtual bool				OnInit();
			virtual bool				OnInitConnectionToDataBase();
			virtual bool				OnInitConnectionToConnector();
			virtual bool				OnInitConnectionToCache();
			
		public:	
			IConnector();

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
			virtual UInt32				GetTotalRecordCount();	

			virtual bool				AllowConnectors(UInt32 count);	

		};

	}
}


