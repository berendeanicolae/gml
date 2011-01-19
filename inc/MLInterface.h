#ifndef __ML_INTERFACE__
#define __ML_INTERFACE__

#include "gml.h"

//-----------------------------------------------------------------------------------------------------------------------
// Generic type definitions
// 


// Standard HASH (MD5) for all MLRecords
typedef struct _MLHash
{
	UInt32	Value[4];
} MLHash;

//-----------------------------------------------------------------------------------------------------------------------
// Specification Interface for the Notifier object classes
// 

class INotifier 
{
public:
	virtual bool Init (void * data)=0;
	virtual bool UnInit()=0;

	virtual bool Notify(char* msg) =0;	
};

//-----------------------------------------------------------------------------------------------------------------------
// Specification Interface for Generic Database Classes

/*
 * The main structure that we will use for fetching database data
 *
 */

enum DbDataType
{
	NULLVAL = 0,
	UINT8VAL,
	UINT16VAL,
	UINT32VAL,
	UINT64VAL,
	RAWPTRVAL,
	BYTESVAL,
	ASCIISTTVAL,
	UNICSTRVAL,
	HASHVAL
};

typedef struct _DbRecord 
{
	UInt32	Type;
	char*	Name;
	union
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

class IDatabase 
{
protected:
	/*
	 * Generic Notifier object for passing messages
	 *  - in the case of this class mostly errors 
	 */
	INotifier *notifier;

public:
	/*
	 * Constructor of the class
	 */
	IDatabase() {};

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
	virtual bool Init (INotifier &notifier, char* Server="", char* Database="", char* Username="", char* Password="", UInt32 Port=0)=0;

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
	virtual bool FetchNextRow (DbRecordVect &VectPtr)=0;

	/*
	 * Usage: fetch a new record after a previous SqlSelect call
	 * Param:
	 *	- INPUT/OUTPUT DbRecordVect **VectPtr: a double pointer to the calee alocated vector of records			
	 *	- INPUT UInt32 RowNr: the row number to be fetched
	 * Return: true/false if there was a record to fetch or not	 
	 */
	virtual bool FetchRowNr (DbRecordVect &VectPtr, UInt32 RowNr)=0;

	/*
	 *Usage: free the calee allocated vector of records given in a FetchRow call
	 *Param:
	 *	- INPUT DbRecordVect* Vect: a pointer to a DbRecordVect to be freed
	 *Return: true/false if the memory free succeded or not
	 */
	virtual bool FreeRow(DbRecordVect &Vect)=0;

	 /*
	  *Usage: insert a new ENTIRE row into the database
	  *Param:
	  *	- INPUT char* Table: the table the data is to be inserted in
	  *	- INPUT DbRecordVect * Vect: a vector of Record objects to be inserted
	  *	Return: true/false if the action succeded or not
	  */
	virtual bool InsertRow (char* Table, DbRecordVect &Vect)=0;

	/*
	  *Usage: insert a new ENTIRE row into the database
	  *Param:
	  *	- INPUT char* Table: the table the data is to be inserted in
	  *	- INPUT char* Fields: a string of fields to be inserted separated by comma
	  *	- INPUT DbRecordVect * Vect: a vector of Record objects to be inserted
	  *	Return: true/false if the action succeded or not
	  */
	virtual bool InsertRow (char* Table, char* Fields, DbRecordVect &Vect)=0;

	/*
	 * Usage: execute a sql update statement 
	 * Param:
	 *  - INPUT char* SqlStatement: the update sql statement
	 *  - INPUT DbRecordVect* WhereVals: the values used to create the where part of the statement
	 *  - INPUT DbRecordVect* UpdateVals: the values used to replace the old values
	 * Return: true/false if the operation succeded or not
	 */
	virtual bool Update (char* SqlStatement, DbRecordVect &WhereVals, DbRecordVect &UpdateVals)=0;
};


//-----------------------------------------------------------------------------------------------------------------------
// Specification Interface for Machine Learning Database Classes

// MLRecords
typedef struct _MLRecord
{
	UInt32	FeatCount;
	double*	Features;
	double	Weight;
	MLHash	Hash;	
	double	Label;
} MLRecord;

class IMLDatabase
{
protected:
	/*
	 * The all mighty notifier object
	 */
	INotifier *notifier;

	/*
	 * The actual database connection object
	 */
	IDatabase *database;

public:	

	/*
	 * Usage: Initialization function
	 * Params:
	 *	- INPUT INotifier *Notifier: notifier object for passing debugging information
	 *			!!! it can be NULL
	 *	- INPUT IDatabase *Database: the database object to work with 
	 *			!!! this object is already initialized but not connected
	 */	
	virtual bool Init(INotifier &Notifier,IDatabase &Database)=0;

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


#endif
