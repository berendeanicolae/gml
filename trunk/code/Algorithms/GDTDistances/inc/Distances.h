#include "GenericDistAlgorithm.h"

struct DistThreadData
{
	GML::ML::MLRecord	SetRec;
};

class Distances: public GenericDistAlgorithm
{
	enum {
		METHOD_PositiveToNegativeDistance = 0,
		METHOD_DistanceTablePositiveToNegative,
		METHOD_DistanceTablePositiveToPositive,
		METHOD_DistanceTableNegativeToNegative,
		METHOD_DistanceTableNegativeToPositive
	};
	enum {
		DIST_FUNC_Manhattan = 0,
		DIST_FUNC_Euclidean,
		DIST_FUNC_Euclidean_Square,
		DIST_FUNC_Minkowski,
		DIST_FUNC_ProcDifference,
	};
	UInt32					Method;
	double					MinDist,MaxDist,Power;
	GML::Utils::GString		DistanceTableFileName;
	GML::Utils::GString		FeaturesWeightFile;
	bool					MergeDistanceTableFiles;
	bool					UseWeightsForFeatures;
	UInt32					DistanceFunction;

	double					*featWeight;

	bool		LoadFeatureWeightFile();
	double		GetDistance(GML::ML::MLRecord &r1,GML::ML::MLRecord &r2);
	bool		ComputePositiveToNegativeDistance(GML::Algorithm::MLThreadData &thData);
	bool		ComputeDistanceTable(GML::Algorithm::MLThreadData &thData,GML::Utils::Indexes &i1,GML::Utils::Indexes &i2,char *Type);
	bool		MergeDistances();
public:
	Distances();
	bool		OnInitThreadData(GML::Algorithm::MLThreadData &thData);
	void		OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);

	bool		OnInit();
	bool		OnCompute();
};