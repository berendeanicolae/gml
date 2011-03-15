#include "GenericVotantPerceptron.h"

//=======================================================================
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
	if ((Order = new UInt32[maxVotes+1])==NULL)
		return false;
	Max = maxVotes;
	Count = 0;
	return true;
}
bool VotantList::Add(PerceptronVector &v,double VoteWeigth)
{
	int		index,el; 
	UInt32	*p1,*p2,*pStart;
	
	if (Count==Max) 
	{
		if (VoteWeigth<Votes[Order[Max-1]].VoteWeigth)
			return false;
	}

	// mut datele la dreapta
	index = FindInsertPoz(VoteWeigth);
	if (Count==Max)
	{
		el = Order[Max-1];
	} else {
		el = Count;
		Count++;
	}

	if (index+1<Count)
	{
		p1 = &Order[Count];
		p2 = p1-1;
		pStart = &Order[index];

		while (p1!=pStart)
		{
			(*p1) = (*p2);
			p2--;
			p1--;
		}		
	}



	Order[index]=el;
	Votes[el].VoteWeigth = VoteWeigth;
	Votes[el].Vector = v;
	
	//printf("Order:");
	//for (int tr=0;tr<Count;tr++)
	//	printf("%4d,",Order[tr]);
	//printf("\nValue:");
	//for (int tr=0;tr<Count;tr++)
	//	printf("%4d,",(int)Votes[Order[tr]].VoteWeigth);
	//printf("\n");

	return true;
}
int  VotantList::FindInsertPoz(double VoteWeigth)
{
	int		start,end,mij;
	double	v;

	if (Count==0)
		return 0;

	start = 0;
	end = ((int)Count) -1;

	while (true)
	{
		if (start>end)
			return start;
		mij = (start+end)/2;
		v = Votes[Order[mij]].VoteWeigth;
		if (VoteWeigth>v)
		{
			end = mij;
			if (start==end)
				return start;
			continue;
		}
		if (VoteWeigth<v)
		{
			start = mij+1;
			continue;
		}
		return mij;
	}
}