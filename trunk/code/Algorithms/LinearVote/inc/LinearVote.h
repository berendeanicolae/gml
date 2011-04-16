#ifndef __LINEAR_VOTE__
#define __LINEAR_VOTE__

#include "gmllib.h"

class PerceptronVector
{
public:
	double				Bias;
	double				*Weight;
	UInt32				Count;
	double				Vote;
	GML::Utils::GString	FileName;
public:
	PerceptronVector();
	PerceptronVector(PerceptronVector &r);
	~PerceptronVector();

	bool operator > (PerceptronVector &r);
	bool operator < (PerceptronVector &r);

	void	Destroy();
	bool	Create(UInt32 count);
};
struct ThreadData
{
	GML::ML::MLRecord				Record;
	GML::Utils::AlgorithmResult		Res;
	GML::Utils::Interval			Range;
	UInt32							eqVotes;
};
class LinearVote: public GML::Algorithm::IMLAlgorithm
{	
	enum {
		PARALLEL_CMD_TEST = 0,
	};

	enum {
		LOAD_VOTES_FROMLIST = 0,
		LOAD_VOTES_FROMWEIGHTPATH,
	};
	enum {
		VOTE_COMPUTE_ADDITION = 0,
		VOTE_COMPUTE_MULTIPLY,
		VOTE_COMPUTE_COUNT,
	};
	enum {
		VOTE_POZITIVE = 0,
		VOTE_NEGATIVE,
	};

	GML::Utils::GString							WeightFiles;
	GML::Utils::GString							WeightPath;
	GML::Utils::GString							VotePropertyName;
	UInt32										VotesLoadingMethod;
	UInt32										VoteComputeMethod;
	UInt32										VoteOnEqual;

public:
	GML::Utils::GTVector<PerceptronVector>		pVectors;
	ThreadData									*ptData;
	GML::Utils::ThreadParalelUnit				*tpu;
	GML::Utils::Indexes							indexes;

protected:
	bool					Create(PerceptronVector &pv,char *fileName);
	bool					LoadVotesFromWeightPath();
	bool					LoadVotesFromList();
	bool					PerformTest(ThreadData &td);
	void					DoTest();
	bool					CheckValidVotes();
public:
	LinearVote();
	
	void					OnRunThreadCommand(UInt32 threadID,UInt32 threadCommand);
	bool					Init();
	void					OnExecute();
};

#endif
