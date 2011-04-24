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
	enum
	{
		SAVE_RESULTS_NONE=0,
		SAVE_RESULTS_TEXT,
		SAVE_RESULTS_PARSABLE
	};
protected:
	GML::Utils::Indexes							indexesPozitive,indexesNegative;
	GML::Utils::GTFVector<CentroidDistances>	distInfo;

	UInt32					SaveResults;
	bool					SortResults;
	UInt32					minimCorectelyClassified;
	GML::Utils::GString		ResultFileName;

	bool					CreatePozitiveAndNegativeIndexes();

	void					OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool					OnInitThreadData(GML::Algorithm::MLThreadData &thData);

	bool					FindCentroid(GML::Algorithm::MLThreadData &thData,GML::Utils::Indexes &indexWork,GML::Utils::Indexes &indexPoz,GML::Utils::Indexes &indexNeg);
	bool					BuildLineRecord(CentroidDistances *cd,GML::Utils::GString &str);
	bool					SaveResultsToDisk();
public:
	Centroid();

	bool					Init();
	void					OnExecute();
};

#endif

