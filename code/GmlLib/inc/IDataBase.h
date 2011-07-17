#pragma once

#include "compat.h"
#include "INotifier.h"
#include "GTFVector.h"
#include "DBRecord.h"
#include "AttributeList.h"
#include "GMLObject.h"


namespace GML
{
	namespace DB
	{
		class EXPORT IDataBase: public GML::Utils::GMLObject
		{
		protected:
			GML::Utils::INotifier						*notifier;
			GML::Utils::GTFVector<GML::DB::ColumnInfo>	Columns;
			GML::Utils::GTFVector<char>					Names;
			UInt32										nrRecords;

			bool										AddColumn(UInt32 DataType,UInt32 ColumnType,char *name);
		public:
			IDataBase();
			bool										Init (GML::Utils::INotifier &notifier, char *connectionString);
			UInt32										GetRecordCount();
			GML::Utils::GTFVector<GML::DB::ColumnInfo>*	GetColumns();


			virtual bool								OnInit()=0;			
			virtual bool								Close()=0;
			virtual bool								BeginIteration() = 0;
			virtual bool								ReadNextRecord(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr) = 0;
		};
	}
}


