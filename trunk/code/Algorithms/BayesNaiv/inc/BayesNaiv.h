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
	UINT32					nrOfIterations;
	double					procToSetInfected;
	double					bestIteration;
	bool					saveTrainedProbs;
	bool					classicMul;
	GML::Utils::GString		pathToProbsFile;	
	GML::Utils::GString		pathTrainedProbsFile;

public:
	BayesNaiv();

	bool	Init();

	bool	PerformTrain();
	bool	PerformTestClassicMul();
	bool	PerformTestNewMul();

	void	OnExecute();
};