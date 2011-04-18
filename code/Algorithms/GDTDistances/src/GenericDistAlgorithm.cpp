#include "GenericDistAlgorithm.h"

//==============================================================================
GenericDistAlgorithm::GenericDistAlgorithm()
{
	ObjectName = "GenericDistance";

	SetPropertyMetaData("Command","!!LIST:None=0,Compute!!");
}
bool	GenericDistAlgorithm::CreatePozitiveAndNegativeIndexes()
{
	UInt32	tr;
	UInt32	pozitiveCount,negativeCount,nrRecords;
	double	label;
	bool	res;

	notif->Info("[%s] -> Creating Pozitive and Negative indexes ... ",ObjectName);
	pozitiveCount = negativeCount = 0;
	nrRecords = con->GetRecordCount();
	
	for (tr=0;tr<nrRecords;tr++)
	{
		if (con->GetRecordLabel(label,tr)==false)
		{
			notif->Error("[%s] -> Unable to read label for record #%d",ObjectName,tr);
			return false;
		}
		if (label==1.0)
			pozitiveCount++;
		else
			negativeCount++;
	}
	notif->Info("[%s] -> Pozitive = %d ,Negative = %d",ObjectName,pozitiveCount,negativeCount);
	if (indexesPozitive.Create(pozitiveCount)==false)
	{
		notif->Error("[%s] -> Unable to alloc %d pozitive indexes",ObjectName,pozitiveCount);
		return false;
	}
	if (indexesNegative.Create(negativeCount)==false)
	{
		notif->Error("[%s] -> Unable to alloc %d negative indexes",ObjectName,negativeCount);
		return false;
	}
	// adaugam si indexii
	for (tr=0;tr<nrRecords;tr++)
	{
		if (con->GetRecordLabel(label,tr)==false)
		{
			notif->Error("[%s] -> Unable to read label for record #%d",ObjectName,tr);
			return false;
		}
		if (label==1.0)
			res = indexesPozitive.Push(tr);
		else
			res = indexesNegative.Push(tr);
		if (res==false)
		{
			notif->Error("[%s] -> Unable to add index #%d",ObjectName,tr);
			return false;
		}		
	}
	notif->Info("[%s] -> Pozitive and Negative indexes created !",ObjectName);
	return true;
}
bool	GenericDistAlgorithm::Init()
{
	if (InitConnections()==false)
		return false;
	if (InitThreads()==false)
		return false;
	if (RecordsStatus.Create(con->GetRecordCount())==false)
	{
		notif->Error("[%s] -> Unable to create Status Record for %d records ",ObjectName,con->GetRecordCount());
		return false;
	}
	if (RecordsStatus.Resize(con->GetRecordCount())==false)
	{
		notif->Error("[%s] -> Unable to alloc Status Record for %d records ",ObjectName,con->GetRecordCount());
		return false;
	}
	memset(RecordsStatus.GetVector(),0,RecordsStatus.Len());
	// curat toate recordurile
	



	return OnInit();
}
bool	GenericDistAlgorithm::OnInit()
{
	return true;
}
bool	GenericDistAlgorithm::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	DistThreadData *d = new DistThreadData();
	if (d==NULL)
		return false;
	if (con->CreateMlRecord(d->SetRec)==false)
		return false;
	thData.Context = d;
	return true;
}
void	GenericDistAlgorithm::OnExecute()
{
	StopAlgorithm = false;
	
	switch (Command)
	{
		case COMMAND_NONE:
			notif->Info("[%s] -> Nothing to do ... ",ObjectName);
			return;
		case COMMAND_COMPUTE:
			OnCompute();
			return;
	};
	notif->Error("[%s] -> Unkwnown command ID : %d",ObjectName,Command);
}
