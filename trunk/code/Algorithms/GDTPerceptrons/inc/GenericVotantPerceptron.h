#ifndef __GENERIC_VOTANT__
#define __GENERIC_VOTANT__

#include "GenericPerceptron.h"

struct VoteInformation
{
	PerceptronVector	Vector;
	double				VoteWeigth;
};
struct VotantList
{
	VoteInformation		*Votes;
	UInt32				*Order;
	UInt32				Count;
	UInt32				Max;
public:
	VotantList();
	~VotantList();
	int					FindInsertPoz(double VoteWeigth);
	bool				Create(UInt32 maxVotes);
	void				Destroy();
	bool				Add(PerceptronVector &v,double VoteWeigth);
};


class GenericVotantPerceptron: public GenericPerceptron
{
};

#endif
