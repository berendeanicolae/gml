#include "GenericDistAlgorithm.h"

struct DistThreadData
{
	GML::ML::MLRecord	SetRec;
};

class Distances: public GenericDistAlgorithm
{
	enum {
		METHOD_PositiveToNegativeDistance = 0,
	};
	UInt32		Method;
	double		MinDist,MaxDist;

	bool		ComputePositiveToNegativeDistance(GML::Algorithm::MLThreadData &thData);
public:
	Distances();
	bool		OnInitThreadData(GML::Algorithm::MLThreadData &thData);
	void		OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);

	bool		OnInit();
	bool		OnCompute();
};