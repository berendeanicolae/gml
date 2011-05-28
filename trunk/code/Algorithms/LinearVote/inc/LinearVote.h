#ifndef __LINEAR_VOTE__
#define __LINEAR_VOTE__

#include "gmllib.h"

class PerceptronVector
{
public:
	double				Bias;
	double				*Weight;
	UInt32				Count;
	double				PositiveVote;
	double				NegativeVote;
	GML::Utils::GString	FileName;
public:
	PerceptronVector();
	PerceptronVector(PerceptronVector &r);
	~PerceptronVector();

	//bool operator > (PerceptronVector &r);
	//bool operator < (PerceptronVector &r);

	void	Destroy();
	bool	Create(UInt32 count);
};
struct LinearVoteThreadData
{
	UInt32	eqVotes;
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
	enum {
		HASH_SELECT_NONE = 0,
		HASH_SELECT_ALL,
		HASH_SELECT_CORECTELY_CLASIFY,
		HASH_SELECT_INCORECTELY_CLASIFY,
		HASH_SELECT_POSITIVE,
		HASH_SELECT_NEGATIVE,
		HASH_SELECT_POSITIVE_CORECTELY_CLASIFY,
		HASH_SELECT_POSITIVE_INCORECTELY_CLASIFY,
		HASH_SELECT_NEGATIVE_CORECTELY_CLASIFY,
		HASH_SELECT_NEGATIVE_INCORECTELY_CLASIFY,
	};

	GML::Utils::GString							WeightFiles;
	GML::Utils::GString							WeightPath;
	GML::Utils::GString							PositiveVotePropertyName;
	GML::Utils::GString							NegativeVotePropertyName;
	UInt32										VotesLoadingMethod;
	UInt32										VoteComputeMethod;
	UInt32										VoteOnEqual;
	UInt32										HashSelectMethod;
	double										PositiveVoteFactor;
	double										NegativeVoteFactor;

public:
	GML::Utils::GTVector<PerceptronVector>		pVectors;
	GML::Utils::GTFVector<UInt8>				RecordsStatus;
	GML::Utils::Indexes							indexes;

protected:
	bool					Create(PerceptronVector &pv,char *fileName);
	bool					LoadVotesFromWeightPath();
	bool					LoadVotesFromList();
	bool					PerformTest(GML::Algorithm::MLThreadData &td);
	void					DoTest();
	bool					CheckValidVotes();	
public:
	LinearVote();
	
	bool					OnInitThreadData(GML::Algorithm::MLThreadData &thData);
	void					OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool					Init();
	void					OnExecute();
};

#endif

