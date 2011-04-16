#ifndef __CACHE_BUILDER__
#define __CACHE_BUILDER__

#include "gmllib.h"

class CacheBuilder: public GML::Algorithm::IMLAlgorithm
{
	GML::Utils::GString		CacheName;
public:
	CacheBuilder();

	bool					Init();
	void					OnExecute();
};

#endif

