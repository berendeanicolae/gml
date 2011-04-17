#include "GenericDistAlgorithm.h"


class Distances: public GenericDistAlgorithm
{
	enum {
		METHOD_PositiveToNegativeDistance = 0,

	};
	UInt32		Method;

	bool	ComputePositiveToNegativeDistance(GML::Algorithm::MLThreadData &thData);
public:
	Distances();

	void	OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);

	bool	OnInit();
	bool	OnCompute();
};