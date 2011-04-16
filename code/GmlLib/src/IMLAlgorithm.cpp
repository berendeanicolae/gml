#include "IMLAlgorithm.h"

void ThreadRedirectFunction(GML::Utils::IParalelUnit *paralel,void *context)
{
	GML::Algorithm::IMLAlgorithm *alg = (GML::Algorithm::IMLAlgorithm *)context;
	alg->OnRunThreadCommand(alg->ThData[paralel->GetID()],paralel->GetCodeID());
}
//=======================================================================================
GML::Algorithm::IMLAlgorithm::IMLAlgorithm()
{
	ObjectName = "IMLAlgorithm";

	db = NULL;
	con = NULL;
	notif = NULL;
	tpu = NULL;
	ThData = NULL;

	LinkPropertyToString("DataBase"					,DataBase				,"","DataBase connection string");
	LinkPropertyToString("Connector"				,Conector				,"","Connector connection string");
	LinkPropertyToString("Notifier"					,Notifier				,"","Notifier plugin");
	LinkPropertyToUInt32("ThreadsCount"				,threadsCount			,1,"Number of threads to be used for parallel computations.");
}

bool GML::Algorithm::IMLAlgorithm::InitConnections()
{
	if ((notif = GML::Builder::CreateNotifier(Notifier.GetText()))==NULL)
		return false;
	if (DataBase.Len()!=0)
	{
		if ((db = GML::Builder::CreateDataBase(DataBase.GetText(),*notif))==NULL)
		{
			notif->Error("[%s] -> Unable to create Database (%s)",ObjectName,DataBase.GetText());
			return false;
		}
		if (db->Connect()==false)
		{
			notif->Error("[%s] -> Unable to connesct to Database (%s)",ObjectName,DataBase.GetText());
			return false;
		}
		if ((con = GML::Builder::CreateConnectors(Conector.GetText(),*notif,*db))==NULL)
		{
			notif->Error("[%s] -> Unable to create Conector (%s)",ObjectName,Conector.GetText());
			return false;
		}
	} else {
		if ((con = GML::Builder::CreateConnectors(Conector.GetText(),*notif))==NULL)
		{
			notif->Error("[%s] -> Unable to create Conector (%s)",ObjectName,Conector.GetText());
			return false;
		}
	}
	return true;
}
bool GML::Algorithm::IMLAlgorithm::InitThreads()
{
	UInt32	tr;

	if (notif==NULL)
	{
		DEBUGMSG("[%s] -> Connection to a notifier is not set ...",ObjectName);
		return false;
	}
	if (con==NULL)
	{
		notif->Error("[%s] -> You need to attached a valid notifier first !",ObjectName);
		return false;
	}
	if (threadsCount<1)
	{
		notif->Error("[%s] -> Invalid number of threads (%d). Should be at least one thread.",threadsCount);
		return false;
	}
	if ((ThData = new GML::Algorithm::MLThreadData[threadsCount])==NULL)
	{
		notif->Error("[%s] -> Unable to create %d MLThreadData Objects",threadsCount,threadsCount);
		return false;
	}
	if ((tpu = new GML::Utils::ThreadParalelUnit[threadsCount])==NULL)
	{
		notif->Error("[%s] -> Unable to create %d threads ",ObjectName,threadsCount);
		return false;
	}
	for (tr=0;tr<threadsCount;tr++)
	{
		if (tpu[tr].Init(tr,this,ThreadRedirectFunction)==false)
		{
			notif->Error("[%s] -> Unable to start thread #%d",ObjectName,tr);
			return false;
		}
		if (con->CreateMlRecord(ThData[tr].Record)==false)
		{
			notif->Error("[%s] -> Unable to create MLThreadData[%d].Record",ObjectName,tr);
			return false;
		}
		ThData[tr].ThreadID = tr; 
		ThData[tr].Res.Clear();
		ThData[tr].Res.time.Start();
		ThData[tr].Range.Set(0,0);
		ThData[tr].Context = NULL;
		if (OnInitThreadData(ThData[tr])==false)
		{
			notif->Error("[%s] -> Unable to create MLThreadData.Context ",ObjectName);
			return false;
		}
	}
	// totul e ok 
	if (OnInitThreads()==false)
	{
		notif->Error("[%s] -> OnInitThreads failed !",ObjectName);
		return false;
	}
	notif->Info("[%s] -> Computation Threads created: %d",ObjectName,threadsCount);
	return true;
}
bool GML::Algorithm::IMLAlgorithm::ExecuteParalelCommand(UInt32 command)
{
	UInt32	tr;

	if ((tpu==NULL) || (threadsCount<1))
	{
		notif->Error("[%s] -> Thread data was not initilized (Did you call InitThreads method ?)",ObjectName);
		return false;
	}
	// executie
	for (tr=0;tr<threadsCount;tr++)
		if (tpu[tr].Execute(command)==false)
		{
			notif->Error("[%s] -> Error on runnig thread #%d",ObjectName,tr);
			return false;
		}
	// asteptare
	for (tr=0;tr<threadsCount;tr++)
		if (tpu[tr].WaitToFinish()==false)
		{
			notif->Error("[%s] -> WaitToFinish failed on thread #%d",ObjectName,tr);
			return false;
		}
	// all ok
	return true;
}
void GML::Algorithm::IMLAlgorithm::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand)
{
}
bool GML::Algorithm::IMLAlgorithm::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	return true;
}
bool GML::Algorithm::IMLAlgorithm::OnInitThreads()
{
	return true;
}
bool GML::Algorithm::IMLAlgorithm::SplitMLThreadDataRange(UInt32 maxCount)
{
	UInt32	tr,splitValue,start;

	if ((threadsCount<1) || (ThData==NULL))
	{
		notif->Error("[%s] -> Unable to split data to range. Have you run InitThreads !",ObjectName);
		return false;
	}

	splitValue = (maxCount/threadsCount);
	start = 0;
	for (tr=0;tr<threadsCount;tr++)
	{
		if ((start+splitValue)>maxCount)
			splitValue = maxCount-start;
		ThData[tr].Range.Set(start,start+splitValue);
		start+=splitValue;
	}

	return true;
}