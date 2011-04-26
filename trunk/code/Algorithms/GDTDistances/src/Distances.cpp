#include "Distances.h"

Distances::Distances()
{
	ObjectName = "Distances";

	LinkPropertyToUInt32("Method",Method,0,"!!LIST:PositiveToNegativeDistance=0!!");
	LinkPropertyToDouble("MinDistance",MinDist,0,"Minimal distance");
	LinkPropertyToDouble("MaxDistance",MaxDist,1,"Maximal distance");

}
bool Distances::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	DistThreadData *d = new DistThreadData();
	if (d==NULL)
		return false;
	if (con->CreateMlRecord(d->SetRec)==false)
		return false;
	thData.Context = d;
	return true;
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
	UInt32				tr,gr,featuresCount,pozitiveCount,negativeCount,idxPoz,idxNeg;
	DistThreadData*		dt = (DistThreadData *)thData.Context;
	double				dist;
	UInt8				*ptr = RecordsStatus.GetVector();

	featuresCount = con->GetFeatureCount();
	pozitiveCount = indexesPozitive.Len();
	negativeCount = indexesNegative.Len();
	if (thData.ThreadID==0)
		notif->StartProcent("[%s] -> Computing ... ",ObjectName);
	for (tr=thData.ThreadID;tr<pozitiveCount;tr+=threadsCount)
	{
		idxPoz = indexesPozitive.Get(tr);
		if (con->GetRecord(thData.Record,idxPoz)==false)
		{
			notif->Error("[%s] -> Unable to read record #%d",ObjectName,idxPoz);
			return false;
		}
		for (gr=0;gr<negativeCount;gr++)
		{
			idxNeg = indexesNegative.Get(gr);
			if (con->GetRecord(dt->SetRec,idxNeg)==false)
			{
				notif->Error("[%s] -> Unable to read record #%d",ObjectName,idxNeg);
				return false;
			}
			dist = GML::ML::VectorOp::PointToPointDistanceSquared(thData.Record.Features,dt->SetRec.Features,featuresCount);
			if ((dist>=MinDist) && (dist<=MaxDist))
			{				
				ptr[idxPoz] = 1;
				ptr[idxNeg] = 1;
			}
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
	switch (Method)
	{
		case METHOD_PositiveToNegativeDistance:
			ComputePositiveToNegativeDistance(thData);
			break;
	}
}
bool Distances::OnCompute()
{
	switch (Method)
	{
		case METHOD_PositiveToNegativeDistance:
			ExecuteParalelCommand(Method);
			SaveHashResult(HashFileName.GetText(),HashFileType,RecordsStatus);
			return true;
	}
	return false;
}
