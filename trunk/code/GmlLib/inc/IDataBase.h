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
			virtual bool				Connect ()=0;
			virtual bool				Disconnect ()=0;
			virtual bool				ExecuteQuery(char* Statement,UInt32 *rowsCount=NULL)=0;
			virtual bool				FetchNextRow (GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)=0;
			virtual bool				GetColumnInformations (GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)=0;

			 /*
			  *Usage: insert a new ENTIRE row into the database
			  *Param:
			  *	- INPUT char* Table: the table the data is to be inserted in
			  *	- INPUT DbRecordVect * Vect: a vector of Record objects to be inserted
			  *	Return: true/false if the action succeded or not
			  */
			virtual bool				InsertRow (char* Table, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect)=0;

			/*
			  *Usage: insert a new ENTIRE row into the database
			  *Param:
			  *	- INPUT char* Table: the table the data is to be inserted in
			  *	- INPUT char* Fields: a string of fields to be inserted separated by comma
			  *	- INPUT DbRecordVect * Vect: a vector of Record objects to be inserted
			  *	Return: true/false if the action succeded or not
			  */
			virtual bool				InsertRow (char* Table, char* Fields, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect)=0;

			/*
			 * Usage: execute a sql update statement 
			 * Param:
			 *  - INPUT char* SqlStatement: the update sql statement
			 *  - INPUT DbRecordVect* WhereVals: the values used to create the where part of the statement
			 *  - INPUT DbRecordVect* UpdateVals: the values used to replace the old values
			 * Return: true/false if the operation succeded or not
			 */
			virtual bool				Update (char* SqlStatement, GML::Utils::GTFVector<GML::DB::DBRecord> &WhereVals, GML::Utils::GTFVector<GML::DB::DBRecord> &UpdateVals)=0;
		};
	}
}


