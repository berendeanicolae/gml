#ifndef __ML_INTERFACE__
#define __ML_INTERFACE__

#include "gml.h"

//-----------------------------------------------------------------------------------------------------------------------

// Standard HASH (MD5) for all MLRecords
typedef struct _MLHash
{
	UInt32	Value[4];
} MLHash;

// MLRecords
typedef struct _MLRecord
{
	UInt32	FeaturesCount;
	double*	Features;
	double	Weight;
	MLHash	Hash;	
	double	Label;
} MLRecord;

class IMLGenericDatabase
{

public:
	// pentru paralelizare o sa dam cel mai probabil la initializare si intervale 
	// aici mai trebuie un pic discutata treaba	
	// nu am inca o interfata pentru notifier , dar o sa il primesti si pe ala , ca sa poti sa
	// notifici si tu daca sunt probleme sau alte chestii 
	
	virtual bool		Init(void *Notifier,char *dbName,char *user="",char *password="",UInt32 port=0)=0;
	virtual bool		Close();
	
	virtual bool		CreateMLRecord(MLRecord &record)=0;
	virtual bool		DestroyMLRecords(MLRecord &record)=0;
	virtual bool		GetRecord(MLRecord &record,UInt32 index)=0;
	virtual UInt32		GetFeaturesCount()=0;
	virtual UInt32		GetRecordsCount()=0;
	
	// daca avem paralelizare , GetRecordsCount() va returna doar recordurile utilizate de unitatea curenta de paraleliare  
	virtual UInt32		GetTotalRecordsCount()=0;
};

//-----------------------------------------------------------------------------------------------------------------------

typedef struct _DbRecord 
{
	UInt32	Type;
	union	Data
	{
		UInt8		UInt8Val;
		UInt16		UInt16Val;
		UInt32		UInt32Val;
		UInt64		UInt64Val;
		void*		RawPtrVal;
		UInt8*		BytesVal;
		char*		AsciiStrVal;
		wchar_t*	UnicStrVal;
		MLHash		HashVal;
	};
} DbRecord, *PDbRecord;

typedef GTVector<DbRecord>	DbRecordVect;

/*
 * mcimpoesu: 15.01.2011
 * - first version of the Database connection interface specification
 *
 */

class IDatabase 
{
public:
	/*
	 * Constructor of the class
	 */
	IDatabase();

	/*
	 * Destructor of the class
	 */
	~IDatabase();

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
	virtual bool Connect (char* Database, char* Username="", char* Password="", UInt32 Port=0)=0;

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
	virtual UInt32 SqlSelect (char* SqlStatement) = 0;

	/*
	 * Usage: emit a sql select statement that is broken in 3 pieces
	 * Param: 
	 *	- INPUT char* What:  what columns to select (it can be "*" for all of them)
	 *  - INPUT char* Where: the sql conditions for the select
	 *  - INPUT char* From:  what table to select from
	 *  Return: the number of rows fetched during statement executution
	 */
	virtual UInt32 SqlSelect (char* What, char* Where, char* From) = 0;
	
	/*
	 * Usage: fetch a new record after a previous SqlSelect call
	 * Param:
	 *	- INPUT/OUTPUT DbRecordVect **VectPtr: a double pointer to the calee alocated vector of records			
	 * Return: true/false if there was a record to fetch or not	 
	 */
	virtual bool FetchRow (DbRecordVect **VectPtr);

	/*
	 * Usage: fetch a new record after a previous SqlSelect call
	 * Param:
	 *	- INPUT/OUTPUT DbRecordVect **VectPtr: a double pointer to the calee alocated vector of records			
	 *	- INPUT UInt32 RowNr: the row number to be fetched
	 * Return: true/false if there was a record to fetch or not	 
	 */
	virtual bool FetchRowNr (DbRecordVect **VectPtr, UInt32 RowNr);

	/*
	 *Usage: free the calee allocated vector of records given in a FetchRow call
	 *Param:
	 *	- INPUT DbRecordVect* Vect: a pointer to a DbRecordVect to be freed
	 *Return: true/false if the memory free succeded or not
	 */
	virtual bool FreeRow(DbRecordVect* Vect);

	 /*
	  *Usage: insert a new ENTIRE row into the database
	  *Param:
	  *	- INPUT char* Table: the table the data is to be inserted in
	  *	- INPUT DbRecordVect * Vect: a vector of Record objects to be inserted
	  *	Return: true/false if the action succeded or not
	  */
	virtual bool InsertRow (char* Table, DbRecordVect* Vect)=0;

	/*
	  *Usage: insert a new ENTIRE row into the database
	  *Param:
	  *	- INPUT char* Table: the table the data is to be inserted in
	  *	- INPUT char* Fields: a string of fields to be inserted separated by comma
	  *	- INPUT DbRecordVect * Vect: a vector of Record objects to be inserted
	  *	Return: true/false if the action succeded or not
	  */
	virtual bool InsertRow (char* Table, char* Fields, DbRecordVect * Vect)=0;

	/*
	 * Usage: execute a sql update statement 
	 * Param:
	 *  - INPUT char* SqlStatement: the update sql statement
	 *  - INPUT DbRecordVect* WhereVals: the values used to create the where part of the statement
	 *  - INPUT DbRecordVect* UpdateVals: the values used to replace the old values
	 * Return: true/false if the operation succeded or not
	 */
	virtual bool Update (char* SqlStatement, DbRecordVect* WhereVals, DbRecordVect* UpdateVals)=0;
};

#endif
