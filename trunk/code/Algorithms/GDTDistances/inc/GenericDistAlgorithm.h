#ifndef __GEN_DIST_ALG__
#define __GEN_DIST_ALG__

#include "gmllib.h"

struct ThreadData
{
	UInt32				ID;
	GML::ML::MLRecord	Rec;

};

class GenericDistAlgorithm : public GML::Algorithm::IAlgorithm
{
protected:
	// proprietati
	UInt32							threadsCount;
	GML::DB::IDataBase				*db;
	GML::ML::IConnector				*con;

	// proprietati	
	GML::Utils::GString				Conector;
	GML::Utils::GString				DataBase;
	GML::Utils::GString				Notifier;

	GML::Utils::ThreadParalelUnit	*tpu;
public:
	ThreadData						*ptData;


public:
	GenericDistAlgorithm();

	virtual void					OnRunThreadCommand(ThreadData &td,UInt32 command) {};
	virtual bool					OnInit();
	bool							Init();
	void							OnExecute();
	
};


#endif

