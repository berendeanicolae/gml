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
	UInt32					Method;
	double					MinDist,MaxDist;
	GML::Utils::GString		DistanceTableFileName;

	bool		ComputePositiveToNegativeDistance(GML::Algorithm::MLThreadData &thData);
	bool		ComputeDistanceTable(GML::Algorithm::MLThreadData &thData,GML::Utils::Indexes &i1,GML::Utils::Indexes &i2);
public:
	Distances();
	bool		OnInitThreadData(GML::Algorithm::MLThreadData &thData);
	void		OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);

	bool		OnInit();
	bool		OnCompute();
};