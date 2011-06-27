#ifndef __MAP_TEMPLATE__
#define __MAP_TEMPLATE__

#include "gmllib.h"

struct RecInfo
{
	UInt32	Index;
	UInt32	SimCount;
	UInt32	NotSimCount;
	double	Label;
	double	SimAverageDist;
	double	NotSimAverageDist;	
	UInt16	ProcAdd;
	UInt16	ProcCount;
};
struct RecDist
{
	UInt32	Index;
	double	Dist;
};
struct ComputePartsInfo
{
	double	SumSame,SumDiff;
	double	CountSame,CountDiff;
};
struct KNNStatThData
{
	GML::Utils::GTFVector<RecDist>	Dist;
	GML::ML::MLRecord				SecRec;
};
class KNNStatistics: public GML::Algorithm::IMLAlgorithm
{
	enum
	{
		Method_UseK = 0,

	};
	enum
	{
		Interval_None = 0,
		Interval_SimilarCount,
		Interval_NotSimilarCount
	};
	enum
	{
		SortDirection_Ascendent=0,
		SortDirection_Descendent,
	};
	enum
	{
		Sort_None = 0,
		Sort_SimilarCount,
		Sort_NotSimilarCount
	};
protected:
	GML::Utils::GTFVector<RecInfo>	rInfo;

	UInt32							K;
	UInt32							Method;
	UInt32							Interval;
	double							MinInterval,MaxInterval;
	UInt32							Sort;
	UInt32							SortDirection;
	//UInt32						columnWidth;
	GML::ML::MLRecord				MainRecord;
	GML::Utils::GString				ResultFileName;

	void					OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool					OnInitThreadData(GML::Algorithm::MLThreadData &thData);

	void					ComputeParts(ComputePartsInfo &cpi,GML::Utils::GTFVector<RecDist> &Dist,UInt32 start,UInt32 end,double label,bool reset);
	bool					ComputeDist(GML::Algorithm::MLThreadData &thData);
	void					Compute();

	bool					CreateRecordInfo(UInt32 index,GML::Utils::GString &str);
	bool					CreateHeaders(GML::Utils::GString &str);
	bool					SaveData();
public:
	KNNStatistics();

	bool					Init();
	void					OnExecute();
};

#endif

