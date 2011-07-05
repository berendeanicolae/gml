#include "GenericDistAlgorithm.h"

struct DistThreadData
{
	GML::ML::MLRecord	SetRec;
};
struct Plan
{
	double	*Weight;
	double	Bias;
	UInt32	Count;
};
struct NodPoint
{
	int index;
	NodPoint *urm;
};
struct ClosestPoints
{
	double		dist;
	//int			firstPoints[2];
	int			*firstPoints;
	NodPoint	*prim;
	int			count;
};
class Distances: public GenericDistAlgorithm
{
	enum {
		METHOD_PositiveToNegativeDistance = 0,
		METHOD_DistanceTablePositiveToNegative,
		METHOD_DistanceTablePositiveToPositive,
		METHOD_DistanceTableNegativeToNegative,
		METHOD_DistanceTableNegativeToPositive,
		METHOD_DistanceToPlan,
		METHOD_ClosestNegativePositive,
	};

	UInt32							Method;
	double							MinDist,MaxDist,Power;
	GML::Utils::GString				DistanceTableFileName;
	GML::Utils::GString				FeaturesWeightFile;
	GML::Utils::GString				PlanFile;
	bool							MergeDistanceTableFiles;
	bool							UseWeightsForFeatures;
	UInt32							DistanceFunction;
	UInt32							ClosestPointArrayLen;
	GML::ML::MLRecord				MainRecord;

	Plan							plan;
	ClosestPoints					*negativePoints;
	ClosestPoints					*positivePoints;	
	UInt32							FeatCount;
	GML::Utils::GTFVector<double>	planDist;
	double							*featWeight;

	bool							LoadFeatureWeightFile();
	double							GetDistance(GML::ML::MLRecord &r1,GML::ML::MLRecord &r2);
	bool							ComputePositiveToNegativeDistance(GML::Algorithm::MLThreadData &thData);
	bool							ComputeDistanceTable(GML::Algorithm::MLThreadData &thData,GML::Utils::Indexes &i1,GML::Utils::Indexes &i2,char *Type);
	bool							ComputeDistanceToPlan(GML::Algorithm::MLThreadData &thData);
	bool							ComputeClosestPositiveNegative(GML::Algorithm::MLThreadData &thData);
	bool							MergeDistancesClosestPositiveNegative();
	bool							UpdateNegativePositive(UINT32 malId, UINT32 cleanId, double dist);
	bool							SaveNegativePositive(char *fileName);
	bool							ExportNewPair(GML::Utils::File *f, char *fileName, UInt32 posId, UInt32 negID, double dist);
	bool							MergeDistances();
	bool							LoadPlan(char *fileName,Plan &p);
	bool							SavePlanDistances();
	bool							InitClosestPointsArray(ClosestPoints **toInit, int len);		
public:
	Distances();
	bool							OnInitThreadData(GML::Algorithm::MLThreadData &thData);
	void							OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);

	bool							OnInit();
	bool							OnCompute();
};