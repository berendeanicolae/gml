#include "gmllib.h"
#include <math.h>

class BayesNaiv: public GML::Algorithm::IMLAlgorithm
{	
	enum {
		COMMAND_NONE = 0,
		COMMAND_TRAIN,
		COMMAND_TEST
	};

	UInt32					procIgnoreFeature;
	UInt32					penaltyForInfFile;
	double					procToSetInfected;
	bool					classicMul;
	GML::Utils::GString		pathToProbsFile;		

public:
	BayesNaiv();

	bool	Init();

	bool	PerformTrain();
	bool	PerformTestClassicMul();
	bool	PerformTestNewMul();

	void	OnExecute();
};