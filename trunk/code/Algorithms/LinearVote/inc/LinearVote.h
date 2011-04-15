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
};
class LinearVote: public GML::Algorithm::IAlgorithm
{	
	GML::Utils::GString							Conector;
	GML::Utils::GString							DataBase;
	GML::Utils::GString							Notifier;
	GML::Utils::GString							WeightFiles;
	GML::Utils::GString							VotePropertyName;
	GML::DB::IDataBase							*db;
	GML::ML::IConnector							*con;
	UInt32										threadsCount;

public:
	GML::Utils::GTVector<PerceptronVector>		pVectors;
	ThreadData									*ptData;
	GML::Utils::ThreadParalelUnit				*tpu;

	bool					Create(PerceptronVector &pv,char *fileName);
	bool					LoadVotes();
public:
	LinearVote();
	void					OnRunThreadCommand(ThreadData &td,UInt32 command);
	bool					Init();
	void					OnExecute();
};

#endif

