#ifndef __MAP_TEMPLATE__
#define __MAP_TEMPLATE__

#include "gmllib.h"

struct SimpleMapOP
{
	unsigned char	Op;
	unsigned int	FeatureIndex[4];
};

struct MapTemplateOp
{
	unsigned char		Op;
	unsigned int		countPositive,countNegative;
	union
	{
		unsigned char	Raw[32];
		unsigned int	FeatureIndex[4];
	} Data;
};

class MapTemplate: public GML::Algorithm::IMLAlgorithm
{
	enum
	{
		OP_AND_2=0,
		OP_XOR_2,
		OP_AND_3,
		OP_XOR_3,
	};
protected:

	UInt32					ClassType;
	UInt32					SaveResults;
	bool					SortResults;
	UInt32					minPositiveElements,minNegativeElements;
	GML::Utils::GString		ResultFileName;
	GML::Utils::GString		MapTemplateFileName;
	GML::Utils::GString		MapTemplatesFileList;
	GML::Utils::GString		MapTemplatesPath;
	GML::Utils::GString		RayPropertyName;
	GML::Utils::GString		VotePropertyName;
	UInt32					MapTemplatesLoadingMethod;

	GML::ML::MLRecord		MainRecord;

	void					OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool					OnInitThreadData(GML::Algorithm::MLThreadData &thData);

	bool					Compute2LevelOps(GML::Algorithm::MLThreadData &thData,unsigned int op);

	void					Compute();
public:
	MapTemplate();

	bool					Init();
	void					OnExecute();
};

#endif

