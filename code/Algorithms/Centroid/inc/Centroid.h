#ifndef __CENTROID__
#define __CENTROID__

#include "gmllib.h"

class Centroid: public GML::Algorithm::IMLAlgorithm
{
	GML::Utils::Indexes		indexesPozitive,indexesNegative;

	bool					CreatePozitiveAndNegativeIndexes();
public:
	Centroid();

	bool					Init();
	void					OnExecute();
};

#endif

