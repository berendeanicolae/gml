#include "gmllib.h"
#include <math.h>

//structura in care iti pui tu ce date vrei sa fie specifice threadului
struct BayesNaivThreadData
{
	unsigned int			totalClean;
	unsigned int			totalInf;	
};

class BayesNaiv: public GML::Algorithm::IMLAlgorithm
{	
	enum {
		COMMAND_NONE = 0,
		COMMAND_TRAIN,
		COMMAND_TEST,
		COMMAND_COMPUTE_NEW_PROBS
	};

	enum{
		FALSE_POSITIVE	= 1,
		FALSE_NEGATIVE,	
		TRUE_NEGATIVE,	
		TRUE_POSITIVE	
	};

	enum{
		BEST_NONE = 0,
		BEST_SE,
		BEST_SP,
		BEST_ACC
	};

	UInt32					procIgnoreFeature;
	UInt32					penaltyForInfFile;
	UINT32					nrOfIterations;
	UINT32					bestClassifMethod;
	double					procToSetInfected;
	double					bestIteration;
	bool					saveBest;
	bool					classicMul;
	GML::Utils::GString		pathToProbsFile;	
	GML::Utils::GString		pathBestProbsFile;

	double					*pFeatCondInf;
	double					*pFeatCondClean;
	double					pFileInf,pFileClean;
	unsigned int			*vcInf;
	unsigned int			*vcClean;		

	UInt8					*recordsClassif;
	UInt8					*vcFNsC;	
	UInt8					*vcFPsC;

	GML::Utils::AlgorithmResult		result;

public:
	BayesNaiv();

	bool	Init();

	void	SaveProbsToFile(GML::Utils::GString filePath);
	void	LoadProbsFromFile();	
	void	SaveProbsIfBetter(double *best);

	bool	ComputeFeaturesFreq(GML::Algorithm::MLThreadData &thData);
	bool	ComputeWrongFeaturesFreq(GML::Algorithm::MLThreadData &thData);
	bool	BuildRecordsClassif(GML::Algorithm::MLThreadData &thData);
	bool	BuildInitialProbabilities();
	bool	PerformTrain();
	bool	PerformTestClassicMul(bool toLoad, bool alloc, int iterNumber=0);
	bool	PerformTestNewMul();
	bool	InitTrain();
	
	int		GetNrTotalFoundFlags();

	void	OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);

	//GDT: functie care se apeleaza la initializare pentru fiecare thread in parte
	bool	OnInitThreadData(GML::Algorithm::MLThreadData &thData);

	void	OnExecute();
};