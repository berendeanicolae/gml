#ifndef __ML_INTERFACE__
#define __ML_INTERFACE__

#include "Compat.h"

// Standard HASH (MD5) for all MLRecords
struct MLHash
{
	UInt32	Value[4];
};

// MLRecords
struct MLRecord
{
	UInt32	FeaturesCount;
	double*	Features;
	double	Weight;
	MLHash	Hash;	
	double	Label;
};

class IMLGenericDataBase
{

public:
	// pentru paralelizare o sa dam cel mai probabil la initializare si intervale 
	// aici mai trebuie un pic discutata treaba	
	// nu am inca o interfata pentru notifier , dar o sa il primesti si pe ala , ca sa poti sa
	// notifici si tu daca sunt probleme sau alte chestii 
	
	bool		Init(void *Notifier,char *dbName,char *user,char *password,UInt32 port) = 0;
	bool		Close();
	
	bool		CreateMLRecord(MLRecord &record) = 0;
	bool		DestroyMLRecords(MLRecord &record) = 0;
	bool		GetRecord(MLRecord &record,UInt32 index) = 0;
	UInt32	GetFeaturesCount() = 0;
	UInt32	GetRecordsCount() = 0;
	// daca avem paralelizare , GetRecordsCount() va returna doar recordurile utilizate de unitatea curenta de paraleliare  
	UInt32	GetTotalRecordsCount() = 0;
};

#endif
