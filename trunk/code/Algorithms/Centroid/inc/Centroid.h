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
	double				Label;
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

	enum {
		HASH_SELECT_NONE = 0,
		HASH_SELECT_ALL,
		HASH_SELECT_CORECTELY_CLASIFY,
		HASH_SELECT_INCORECTELY_CLASIFY,
		HASH_SELECT_POSITIVE,
		HASH_SELECT_NEGATIVE,
		HASH_SELECT_POSITIVE_CORECTELY_CLASIFY,
		HASH_SELECT_POSITIVE_INCORECTELY_CLASIFY,
		HASH_SELECT_NEGATIVE_CORECTELY_CLASIFY,
		HASH_SELECT_NEGATIVE_INCORECTELY_CLASIFY,
	};
protected:
	GML::Utils::Indexes							indexesPozitive,indexesNegative;
	GML::Utils::GTFVector<CentroidDistances>	distInfo;
	GML::Utils::GTVector<CentroidData>			cVectors;
	GML::Utils::GTFVector<UInt8>				RecordsStatus;

	UInt32					ClassType;
	UInt32					SaveResults;
	bool					SortResults;
	UInt32					HashSelectMethod;
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
	bool					PerformSimpleTest(GML::Algorithm::MLThreadData &td);
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

