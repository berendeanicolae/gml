#include "Distances.h"

Distances::Distances()
{
	ObjectName = "Distances";

	LinkPropertyToUInt32("Method",Method,0,"!!LIST:PositiveToNegativeDistance=0!!");
}
bool Distances::OnInit()
{
	if (Method==METHOD_PositiveToNegativeDistance)
	{
		if (CreatePozitiveAndNegativeIndexes()==false)
			return false;
	}
	return true;
}
bool Distances::ComputePositiveToNegativeDistance(GML::Algorithm::MLThreadData &thData)
{
	UInt32				tr,gr,featuresCount,pozitiveCount,negativeCount;
	DistThreadData*		dt = (DistThreadData *)thData.Context;
	double				dist;

	featuresCount = con->GetFeatureCount();
	pozitiveCount = indexesPozitive.Len();
	negativeCount = indexesNegative.Len();
	if (thData.ThreadID==0)
		notif->StartProcent("[%s] -> Computing ... ",ObjectName);
	for (tr=thData.ThreadID;tr<pozitiveCount;tr+=threadsCount)
	{
		if (con->GetRecord(thData.Record,indexesPozitive.Get(tr))==false)
		{
			notif->Error("[%s] -> Unable to read record #%d",ObjectName,indexesPozitive.Get(tr));
			return false;
		}
		for (gr=0;gr<negativeCount;gr++)
		{
			if (con->GetRecord(dt->SetRec,indexesNegative.Get(tr))==false)
			{
				notif->Error("[%s] -> Unable to read record #%d",ObjectName,indexesNegative.Get(tr));
				return false;
			}
			dist = GML::ML::VectorOp::PointToPointDistance(thData.Record.Features,dt->SetRec.Features,featuresCount);
		}
		if (thData.ThreadID==0)
			notif->SetProcent(tr,pozitiveCount);
	}
	if (thData.ThreadID==0)
		notif->EndProcent();
	return true;
}
void Distances::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand)
{
	ComputePositiveToNegativeDistance(thData);
}
bool Distances::OnCompute()
{
	ExecuteParalelCommand(0);
	return false;
}
