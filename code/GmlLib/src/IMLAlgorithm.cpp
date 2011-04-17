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
void GML::Algorithm::IMLAlgorithm::AddHashSavePropery()
{
	LinkPropertyToString("HashFileName"				,HashFileName			,"","Name of the file with the record hash list result.");
	LinkPropertyToUInt32("HashStoreMethod"			,HashStoreMethod		,0,"!!LIST:Text=0,Binary!!");
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
bool GML::Algorithm::IMLAlgorithm::SaveHashResult(char *fname,UInt32 method,GML::Utils::BitSet &bs,UInt32 CacheSize)
{
	GML::Utils::File		f;
	GML::Utils::Vector		Cache;
	UInt8					*p;
	UInt32					recSize,tr,count,totalRecords;
	GML::DB::RecordHash		rHash;
	GML::Utils::GString		temp;

	switch (method)
	{
		case SAVE_HASHES_AS_TEXT:
			recSize = 32+1;
			break;
		case SAVE_HASHES_AS_BINARY:
			recSize = 16;
			break;
		default:
			notif->Error("[%s] -> Unknown HashStoreMethod = %d",ObjectName,method);
			return false;
	};
	count = bs.CountElements(true);
	if (count==0)
	{
		notif->Error("[%s] -> No elements selected in the BitSet",ObjectName);
		return false;
	}
	if ((fname==NULL) || (fname[0]==0))
	{
		notif->Error("[%s] -> You need to set 'HashFileName' property with the name of the file where you want to save your results",ObjectName);
		return false;
	}
	if (f.Create(fname)==false)
	{
		notif->Error("[%s] -> Unable to create %s",ObjectName,fname);
		return false;
	}
	totalRecords = bs.Len();
	while (true)
	{
		if (Cache.Create(CacheSize,recSize)==false)
		{
			notif->Error("[%s] -> Unable to allocate %d bytes for caching ...",ObjectName,recSize * CacheSize);
			break;
		}
		// pentru binary method , scriu cateva date
		if (method==SAVE_HASHES_AS_BINARY)
		{
			if (f.Write("HASHLIST",8)==false)
				break;
			if (f.Write(&count,sizeof(UInt32))==false)
				break;
		}
		for (tr=0;tr<totalRecords;tr++)
		{
			if (bs.Get(tr))
			{
				// scriu datele
				if (con->GetRecordHash(rHash,tr)==false)
				{
					notif->Error("[%s] -> Unable to read hash for record #%d",ObjectName,tr);
					break;
				}
				switch (method)
				{
					case SAVE_HASHES_AS_TEXT:
						rHash.ToString(temp);
						temp.Add("\n");
						p = (UInt8 *)temp.GetText();
						break;
					case SAVE_HASHES_AS_BINARY:
						recSize = 16;
						p = (UInt8 *)&rHash.Hash.bValue[0];
						break;
				};
				if (Cache.GetSize()>=CacheSize)
				{
					if (f.Write(Cache.GetVector(),Cache.GetSize()*recSize)==false)
						break;
					if (Cache.Resize(0)==false)
						break;
				}
				if (Cache.Push(p)==false)
					break;
			}
		}
		// daca nu am ajuns la final , am o eroare
		if (tr!=totalRecords)
			break;
		if (Cache.GetSize()>=0)
		{
			if (f.Write(Cache.GetVector(),Cache.GetSize()*recSize)==false)
				break;
		}
		f.Close();
		notif->Info("[%s] -> Hashes saved ok in %s (%d hashesh)",ObjectName,fname,count);
		return true;
	}
	f.Close();
	DeleteFileA(fname);
	notif->Error("[%s] -> Unable to write hashes to %s",ObjectName,fname);
	return false;
}