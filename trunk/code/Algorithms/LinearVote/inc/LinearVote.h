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
	~PerceptronVector();

	void	Destroy();
	bool	Create(UInt32 count);
};

class LinearVote: public GML::Algorithm::IAlgorithm
{	
	GML::Utils::GString		Conector;
	GML::Utils::GString		DataBase;
	GML::Utils::GString		Notifier;
	GML::Utils::GString		WeightFiles;
	GML::Utils::GString		VotePropertyName;
	GML::DB::IDataBase		*db;
	GML::ML::IConnector		*con;

	PerceptronVector		*pVectors;


	bool					Create(PerceptronVector &pv,char *fileName);
	bool					LoadVotes();
public:
	LinearVote();

	bool					Init();
	void					OnExecute();
};

#endif

