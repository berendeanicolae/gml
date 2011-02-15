#pragma once

#include "Compat.h"
#include "INotify.h"
#include "MLRecord.h"
#include "IDataBase.h"

#define RECORDS_TABLE_NAME			"RecordTable"
#define FEATURES_COL_PREFIX			"Feat"
#define HASH_COL_NAME				"Hash"
#define LABEL_COL_NAME				"Label"

#define MAX_SQL_QUERY_SIZE			2048

namespace GML
{
	namespace ML
	{
		class EXPORT IConector
		{
		protected:
			GML::Utils::INotify			*notifier;
			GML::DB::IDataBase			*database;
			GML::ML::IConector			*conector;

		public:	
			IConector();

			virtual bool				OnInit() = 0;
			virtual bool				Init(GML::Utils::INotify &Notifier,GML::DB::IDataBase &Database);
			virtual bool				Init(GML::ML::IConector &conector);

			/*	 
			 * Usage: uninit stuff
			 */
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
			virtual bool GetRecord(MLRecord &record,UInt32 index)=0;

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


