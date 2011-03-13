#include "GenericVotantPerceptron.h"

VotantList::VotantList()
{
	Votes = NULL;
	Order = NULL;
	Count = 0;
	Max = 0;
}
VotantList::~VotantList()
{
	Destroy();
}
void VotantList::Destroy()
{
	if (Votes)
		delete Votes;
	if (Order)
		delete Order;
	Votes = NULL;
	Order = NULL;
	Count = 0;
	Max = 0;
}
bool VotantList::Create(UInt32 maxVotes)
{
	Destroy();
	if (maxVotes<1)
		return false;
	// +1 pentru cand se face sort prin insertie
	if ((Votes = new VoteInformation[maxVotes+1])==NULL)
		return false;
	if ((Order = new UInt32[maxVotes])==NULL)
		return false;
	Max = maxVotes;
	Count = 0;
	return true;
}
bool VotantList::CouldAddVoteWeight(double VoteWeigth)
{
	if (Count<Max)
		return true;
	if (VoteWeigth>Votes[Order[Max-1]].VoteWeigth)
		return true;
	return false;
}
bool VotantList::Add(PerceptronVector &v,double VoteWeigth)
{
	// caut Vote
	if (Count<Max)
	{
		// adaug pe ultima pozitie
	} else {

	}
}
