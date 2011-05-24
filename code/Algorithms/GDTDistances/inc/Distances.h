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
class Distances: public GenericDistAlgorithm
{
	enum {
		METHOD_PositiveToNegativeDistance = 0,
		METHOD_DistanceTablePositiveToNegative,
		METHOD_DistanceTablePositiveToPositive,
		METHOD_DistanceTableNegativeToNegative,
		METHOD_DistanceTableNegativeToPositive,
		METHOD_DistanceToPlan,
	};

	UInt32							Method;
	double							MinDist,MaxDist,Power;
	GML::Utils::GString				DistanceTableFileName;
	GML::Utils::GString				FeaturesWeightFile;
	GML::Utils::GString				PlanFile;
	bool							MergeDistanceTableFiles;
	bool							UseWeightsForFeatures;
	UInt32							DistanceFunction;
	
	Plan							plan;
	GML::Utils::GTFVector<double>	planDist;
	double							*featWeight;

	bool							LoadFeatureWeightFile();
	double							GetDistance(GML::ML::MLRecord &r1,GML::ML::MLRecord &r2);
	bool							ComputePositiveToNegativeDistance(GML::Algorithm::MLThreadData &thData);
	bool							ComputeDistanceTable(GML::Algorithm::MLThreadData &thData,GML::Utils::Indexes &i1,GML::Utils::Indexes &i2,char *Type);
	bool							ComputeDistanceToPlan(GML::Algorithm::MLThreadData &thData);
	bool							MergeDistances();
	bool							LoadPlan(char *fileName,Plan &p);
	bool							SavePlanDistances();
public:
	Distances();
	bool							OnInitThreadData(GML::Algorithm::MLThreadData &thData);
	void							OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);

	bool							OnInit();
	bool							OnCompute();
};