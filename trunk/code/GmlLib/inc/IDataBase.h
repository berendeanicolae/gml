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
			GML::Utils::INotifier			*notifier;

		public:
			virtual ~IDataBase();
			bool						Init (GML::Utils::INotifier &notifier, char *connectionString);
			virtual bool				OnInit()=0;			
			virtual bool				GetRecordsCount(UInt32 &count) = 0;
			virtual bool				BeginIteration(GML::Utils::GTFVector<GML::DB::DBRecord> &ColumnsInfo) = 0;
			virtual bool				ReadNextRecord(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr) = 0;
		};
	}
}


