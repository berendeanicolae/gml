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
class CentroidData
{
public:
	double				*Center;
	UInt32				Count;
	double				Vote;
	double				Ray;
	GML::Utils::GString	FileName;
public:
	CentroidData();
	CentroidData(CentroidData &);
	~CentroidData();

	bool operator > (CentroidData &r);
	bool operator < (CentroidData &r);

	void	Destroy();
	bool	Create(UInt32 count);
};
class Centroid: public GML::Algorithm::IMLAlgorithm
{
	enum
	{
		SAVE_RESULTS_NONE=0,
		SAVE_RESULTS_TEXT,
		SAVE_RESULTS_PARSABLE
	};
	enum
	{
		CLASSTYPE_POSITIVE=0,
		CLASSTYPE_NEGATIVE,
		CLASSTYPE_BOTH
	};
	enum {
		LOAD_CENTROIDS_FROMLIST = 0,
		LOAD_CENTROIDS_FROMWEIGHTPATH,
	};
protected:
	GML::Utils::Indexes							indexesPozitive,indexesNegative;
	GML::Utils::GTFVector<CentroidDistances>	distInfo;
	GML::Utils::GTVector<CentroidData>			cVectors;

	UInt32					ClassType;
	UInt32					SaveResults;
	bool					SortResults;
	UInt32					minPositiveElements,minNegativeElements;
	GML::Utils::GString		ResultFileName;
	GML::Utils::GString		CentroidFileName;
	GML::Utils::GString		CentroidsFileList;
	GML::Utils::GString		CentroidsPath;
	GML::Utils::GString		RayPropertyName;
	GML::Utils::GString		VotePropertyName;
	UInt32					CentroidsLoadingMethod;

	GML::ML::MLRecord		MainRecord;

	bool					CreatePozitiveAndNegativeIndexes();
	bool					Create(CentroidData &pv,char *fileName);
	bool					LoadCentroidsFromPath();
	bool					LoadCentroidsFromList();

	void					OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool					OnInitThreadData(GML::Algorithm::MLThreadData &thData);

	bool					FindCentroid(GML::Algorithm::MLThreadData &thData,GML::Utils::Indexes &indexWork,GML::Utils::Indexes &indexPoz,GML::Utils::Indexes &indexNeg);
	bool					BuildHeaders(GML::Utils::GString &str);
	bool					BuildLineRecord(CentroidDistances *cd,GML::Utils::GString &str);
	bool					SaveResultsToDisk();
	bool					SaveCentroids();
	bool					SaveCentroid(CentroidDistances *cd,char *fileName);

	void					Compute();
	bool					Test();
public:
	Centroid();

	bool					Init();
	void					OnExecute();
};

#endif

