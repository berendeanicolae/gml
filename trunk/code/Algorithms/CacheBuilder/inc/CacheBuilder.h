#ifndef __CACHE_BUILDER__
#define __CACHE_BUILDER__

#include "gmllib.h"

class CacheBuilder: public GML::Algorithm::IAlgorithm
{
	GML::Utils::GString		CacheName;
	GML::Utils::GString		Conector;
	GML::Utils::GString		DataBase;
	GML::Utils::GString		Notifier;
	GML::DB::IDataBase		*db;
	GML::ML::IConnector		*con;

public:
	CacheBuilder();

	bool					Init();
	void					OnExecute();
};

#endif

