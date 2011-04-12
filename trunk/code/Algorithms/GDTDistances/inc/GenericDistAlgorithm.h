#ifndef __GEN_DIST_ALG__
#define __GEN_DIST_ALG__

#include "gmllib.h"

struct ThreadData
{
	UInt32				ID;
	GML::ML::MLRecord	Rec1,Rec2;

};

#define SAVE_CACHE_SIZE		1024

class GenericDistAlgorithm : public GML::Algorithm::IAlgorithm
{
protected:
	enum {
		COMMAND_NONE = 0,
		COMMAND_COMPUTE
	};
	enum {
		SAVE_HASHES_AS_TEXT = 0,
		SAVE_HASHES_AS_BINARY
	};

	// proprietati
	UInt32							threadsCount;
	GML::DB::IDataBase				*db;
	GML::ML::IConnector				*con;

	// proprietati	
	GML::Utils::GString				Conector;
	GML::Utils::GString				DataBase;
	GML::Utils::GString				Notifier;
	GML::Utils::GString				HashFileName;
	UInt32							HashStoreMethod;

	GML::Utils::ThreadParalelUnit	*tpu;
	GML::Utils::Indexes				indexesPozitive,indexesNegative;
	GML::Utils::GTFVector<UInt8>	RecordsStatus;
public:
	ThreadData						*ptData;

	
public:
	GenericDistAlgorithm();


	virtual void					OnRunThreadCommand(ThreadData &td,UInt32 command) {};
	virtual bool					OnInit();
	virtual bool					OnCompute();

	bool							SaveHashResult();
	bool							ExecuteParalelCommand(UInt32 command);
	bool							Init();
	void							OnExecute();
	bool							CreatePozitiveAndNegativeIndexes();
};


#endif

