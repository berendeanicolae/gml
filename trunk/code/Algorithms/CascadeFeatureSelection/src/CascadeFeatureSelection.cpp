#include "CascadeFeatureSelection.h"

CascadeFeatureSelection::CascadeFeatureSelection()
{
	ObjectName = "CascadeFeatureSelection";
}
bool CascadeFeatureSelection::OnProcessRecord(UInt32 recordIndex,GML::Algorithm::MLThreadData &thData)
{
	if (thData.Record.Features[featToRemove]!=0.0)
	{
		RemovedRecords[recordIndex] = true;
	}
	return true;
}
void CascadeFeatureSelection::OnCompute()
{
	UInt32				tr,gr;
	GML::Utils::File	out;
	
	GML::Utils::GString tmp;
	
	if (out.Create(ResultFileName.GetText(),true)==false)
	{
		notif->Error("[%s] -> Unable to create: %s",ObjectName,ResultFileName.GetText());
		return;
	}
	tmp.Create(2048);	
	workingRecordsCount = con->GetRecordCount();

	ClearRemovedRecordsList();
	ClearRemovedFeaturesList();
	
	while (true)
	{
		ComputeScoresAndSort();
		if (FeatScores.Len()==0)
		{
			notif->Info("[%s] -> No more features left ... ending",ObjectName);
			break;
		}
		featToRemove = FeatScores[0].Index;
		RemovedFeatures[featToRemove] = true;
		ProcessRecords();
				
		// afisez
		tmp.SetFormated("%d|Scor:%lf|Poz:%d|Neg:%d|TotalPoz:%d|TotalNeg:%d\n",
						featToRemove,
						FeatScores[0].Score,
						FeatCounters[featToRemove].CountPozitive,
						FeatCounters[featToRemove].CountNegative,
						FeatCounters[featToRemove].CountTotalPozitive,
						FeatCounters[featToRemove].CountTotalNegative);
		
		out.Write(tmp.GetText(),tmp.Len());
	}	
	out.Close();
}