#pragma once

#include "compat.h"
#include "INotify.h"
#include "GTVector.h"
#include "DBRecord.h"


namespace GML
{
	namespace DB
	{
		class EXPORT IDatabase 
		{
		protected:
			/*
			 * Generic Notifier object for passing messages
			 *  - in the case of this class mostly errors 
			 */
			GML::Utils::INotify *notifier;

		public:
			/*
			 * Constructor of the class
			 */
			IDatabase();

			/*
			 * Destructor of the class
			 */
			~IDatabase() {};

			/*
			 *Usage: specify initialization parameters
			 *Param:
			 *	- INPUT INotifier * notifier: notifier object where all messages can be sent (pay attention, it can be NULL)
			 *      - INPUT OPT char* Server: ip address of a remote server
			 *	- INPUT OPT char* Database: the database name to connect to
			 *	- INPUT OPT char* Username: the username credential
			 *	- INPUT OPT char* Password: the password credential
			 *	- INPUT OPT UInt Port: an optional parameter that specified the port 
			 */
			virtual bool Init (GML::Utils::INotify &notifier, char *connectionString)=0;

			/*
			 * Usage: 
			 * - connect to the desired database	 
			 * Param:
			 *	- INPUT char* Database: the database name to connect to
			 *	- INPUT OPT char* Username: the username credential
			 *	- INPUT OPT char* Password: the password credential
			 *	- INPUT OPT UInt Port: an optional parameter that specified the port 
			 *	Return: true/false if we have a connection or not
			 */
			virtual bool Connect ()=0;

			/*
			 * Usage: Disconnect from the database
			 * Return: true/false if the operation succeded or not
			 */
			virtual bool Disconnect ()=0;

			/*
			 * Usage: emit a sql select statement to fetch new data
			 * Param: 
			 *	- INPUT char* SqlStatement: the sql select statement
			 * Return: the number of records fetched during the statement execution
			 */
			virtual UInt32 Select (char* Statement="*")=0;

			/*
			 * Usage: emit a sql select statement that is broken in 3 pieces
			 * Param: 
			 *	- INPUT char* What:  what columns to select (it can be "*" for all of them)
			 *  - INPUT char* Where: the sql conditions for the select
			 *  - INPUT char* From:  what table to select from
			 *  Return: the number of rows fetched during statement executution
			 */
			virtual UInt32 SqlSelect (char* What="*", char* Where="", char* From="")=0;
	
			/*
			 * Usage: fetch a new record after a previous SqlSelect call
			 * Param:
			 *	- INPUT/OUTPUT DbRecordVect **VectPtr: a double pointer to the calee alocated vector of records			
			 * Return: true/false if there was a record to fetch or not	 
			 */
			virtual bool FetchNextRow (GML::Utils::GTVector<GML::DB::DBRecord> &VectPtr)=0;

			/*
			 * Usage: fetch a new record after a previous SqlSelect call
			 * Param:
			 *	- INPUT/OUTPUT DbRecordVect **VectPtr: a double pointer to the calee alocated vector of records			
			 *	- INPUT UInt32 RowNr: the row number to be fetched
			 * Return: true/false if there was a record to fetch or not	 
			 */
			virtual bool FetchRowNr (GML::Utils::GTVector<GML::DB::DBRecord> &VectPtr, UInt32 RowNr)=0;

			/*
			 *Usage: free the calee allocated vector of records given in a FetchRow call
			 *Param:
			 *	- INPUT DbRecordVect* Vect: a pointer to a DbRecordVect to be freed
			 *Return: true/false if the memory free succeded or not
			 */
			virtual bool FreeRow(GML::Utils::GTVector<GML::DB::DBRecord> &Vect)=0;

			 /*
			  *Usage: insert a new ENTIRE row into the database
			  *Param:
			  *	- INPUT char* Table: the table the data is to be inserted in
			  *	- INPUT DbRecordVect * Vect: a vector of Record objects to be inserted
			  *	Return: true/false if the action succeded or not
			  */
			virtual bool InsertRow (char* Table, GML::Utils::GTVector<GML::DB::DBRecord> &Vect)=0;

			/*
			  *Usage: insert a new ENTIRE row into the database
			  *Param:
			  *	- INPUT char* Table: the table the data is to be inserted in
			  *	- INPUT char* Fields: a string of fields to be inserted separated by comma
			  *	- INPUT DbRecordVect * Vect: a vector of Record objects to be inserted
			  *	Return: true/false if the action succeded or not
			  */
			virtual bool InsertRow (char* Table, char* Fields, GML::Utils::GTVector<GML::DB::DBRecord> &Vect)=0;

			/*
			 * Usage: execute a sql update statement 
			 * Param:
			 *  - INPUT char* SqlStatement: the update sql statement
			 *  - INPUT DbRecordVect* WhereVals: the values used to create the where part of the statement
			 *  - INPUT DbRecordVect* UpdateVals: the values used to replace the old values
			 * Return: true/false if the operation succeded or not
			 */
			virtual bool Update (char* SqlStatement, GML::Utils::GTVector<GML::DB::DBRecord> &WhereVals, GML::Utils::GTVector<GML::DB::DBRecord> &UpdateVals)=0;
		};
	}
}


