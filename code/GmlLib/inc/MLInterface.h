#pragma once

#include "Compat.h"
#include "INotify.h"
#include "MLRecord.h"
#include "IDataBase.h"

//typedef GTVector<DbRecord>	DbRecordVect;

namespace GML
{
	namespace ML
	{
		class IConector
		{
		protected:
			/*
			 * The all mighty notifier object
			 */
			GML::Utils::INotify *notifier;

			/*
			 * The actual database connection object
			 */
			GML::DB::IDataBase *database;

		public:	

			/*
			 * Usage: Initialization function
			 * Params:
			 *	- INPUT INotifier *Notifier: notifier object for passing debugging information
			 *			!!! it can be NULL
			 *	- INPUT IDatabase *Database: the database object to work with 
			 *			!!! this object is already initialized but not connected
			 */	
			virtual bool Init(GML::Utils::INotify &Notifier,GML::DB::IDataBase &Database)=0;

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


