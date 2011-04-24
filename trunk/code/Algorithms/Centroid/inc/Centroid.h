#ifndef __CENTROID__
#define __CENTROID__

#include "gmllib.h"

struct CentroidDistances
{
	UInt32	index;
	UInt32	corectelyClasify;
	double	MostDistantSimilarDistance;
	double	ClosestDifferentDistance;
};
struct CentroidThreadData
{
	GML::ML::MLRecord				SecRec;
};

class Centroid: public GML::Algorithm::IMLAlgorithm
{
	GML::Utils::Indexes							indexesPozitive,indexesNegative;
	GML::Utils::GTFVector<CentroidDistances>	distInfo;

	bool					CreatePozitiveAndNegativeIndexes();

	void					OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool					OnInitThreadData(GML::Algorithm::MLThreadData &thData);

	bool					FindCentroid(GML::Algorithm::MLThreadData &thData,GML::Utils::Indexes &indexWork,GML::Utils::Indexes &indexPoz,GML::Utils::Indexes &indexNeg);

public:
	Centroid();

	bool					Init();
	void					OnExecute();
};

#endif

