#include "gmllib.h"

struct KNN_INFO
{
	UInt32	indexElement;
	double	Label;
	double	Dist;
	bool	operator <(KNN_INFO &k1);
	bool	operator >(KNN_INFO &k2);

};

class KNN: public GML::Algorithm::IMLAlgorithm
{
	enum
	{
		DIST_EUCLIDIANA = 0,
		DIST_ZUGGY,
	};
	enum 
	{
		None = 0,
		Test
	};

	GML::Utils::Indexes		memIndex,testIndex;

	GML::ML::MLRecord		ob1,ob2;

	UInt32					k;
	UInt32					threadsCount;
	UInt32					distMethod;
	double					procMem;

	
public:
	KNN();

	bool	Init();
	void	OnExecute();
};