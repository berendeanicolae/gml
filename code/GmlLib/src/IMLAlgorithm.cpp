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

	con = NULL;
	notif = NULL;
	tpu = NULL;
	ThData = NULL;

	LinkPropertyToString("Connector"				,Conector				,"","Connector connection string");
	LinkPropertyToString("Notifier"					,Notifier				,"","Notifier plugin");
	LinkPropertyToUInt32("ThreadsCount"				,threadsCount			,1,"Number of threads to be used for parallel computations.");

}
void GML::Algorithm::IMLAlgorithm::AddHashSaveProperties()
{
	LinkPropertyToString("HashFileName"				,HashFileName			,"","Name of the file with the record hash list result.");
	LinkPropertyToUInt32("HashFileType"				,HashFileType			,0,"!!LIST:Text=0,Binary!!");
}
void GML::Algorithm::IMLAlgorithm::AddDistanceProperties()
{
	LinkPropertyToUInt32("DistanceFunction"			,DistanceFunction		,GML::Algorithm::Metrics::Manhattan,"!!LIST:Manhattan=0,Euclidean,EuclideanSquared,Minkowski,ProcDifference,Binomial,Radial,Sigmoid,HyperbolicTangent!!");	
	LinkPropertyToDouble("DistancePower"			,DistancePower			,1,"Power parameter for distance functions");
	LinkPropertyToDouble("DistanceSigma"			,DistanceSigma			,1,"Sigma parameter for radial function");
	LinkPropertyToDouble("DistanceBias"				,DistanceBias			,0,"Bias parameter for radial function");
	LinkPropertyToDouble("DistanceK"				,DistanceK				,0,"K parameter for hyperbolic function");
}
bool GML::Algorithm::IMLAlgorithm::InitConnections()
{
	if ((notif = GML::Builder::CreateNotifier(Notifier.GetText()))==NULL)
		return false;
	//notif->Error("[%s] -> InitConnections()",ObjectName);
	if ((con = GML::Builder::CreateConnector(Conector.GetText(),*notif))==NULL)
	{
		notif->Error("[%s] -> Unable to create Conector (%s)",ObjectName,Conector.GetText());
		return false;
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

	splitValue = (maxCount/threadsCount)+1;
	start = 0;
	for (tr=0;tr<threadsCount;tr++)
	{
		if ((start+splitValue)>=maxCount)
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
		case HASH_FILE_TEXT:
			recSize = 32+1;
			break;
		case HASH_FILE_BINARY:
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
		if (method==HASH_FILE_BINARY)
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
					case HASH_FILE_TEXT:
						rHash.ToString(temp);
						temp.Add("\n");
						p = (UInt8 *)temp.GetText();
						break;
					case HASH_FILE_BINARY:
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
bool GML::Algorithm::IMLAlgorithm::SaveHashResult(char *fname,UInt32 method,GML::Utils::GTFVector<UInt8> &v,UInt32 CacheSize)
{
	GML::Utils::File		f;
	GML::Utils::Vector		Cache;
	UInt8					*p,*ptr;
	UInt32					recSize,tr,count,totalRecords;
	GML::DB::RecordHash		rHash;
	GML::Utils::GString		temp;

	switch (method)
	{
		case HASH_FILE_TEXT:
			recSize = 32+1;
			break;
		case HASH_FILE_BINARY:
			recSize = 16;
			break;
		default:
			notif->Error("[%s] -> Unknown HashStoreMethod = %d",ObjectName,method);
			return false;
	};
	p = v.GetVector();
	totalRecords = v.Len();
	for (count=0,tr=0;tr<totalRecords;tr++,p++)
	{
		if ((*p)!=0)
			count++;
	}	
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
	while (true)
	{
		if (Cache.Create(CacheSize,recSize)==false)
		{
			notif->Error("[%s] -> Unable to allocate %d bytes for caching ...",ObjectName,recSize * CacheSize);
			break;
		}
		// pentru binary method , scriu cateva date
		if (method==HASH_FILE_BINARY)
		{
			if (f.Write("HASHLIST",8)==false)
				break;
			if (f.Write(&count,sizeof(UInt32))==false)
				break;
		}
		for (tr=0,ptr=v.GetVector();tr<totalRecords;tr++,ptr++)
		{
			if ((*ptr)!=0)
			{
				// scriu datele
				if (con->GetRecordHash(rHash,tr)==false)
				{
					notif->Error("[%s] -> Unable to read hash for record #%d",ObjectName,tr);
					break;
				}
				switch (method)
				{
					case HASH_FILE_TEXT:
						rHash.ToString(temp);
						temp.Add("\n");
						p = (UInt8 *)temp.GetText();
						break;
					case HASH_FILE_BINARY:
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
double GML::Algorithm::IMLAlgorithm::GetDistance(GML::ML::MLRecord &r1,GML::ML::MLRecord &r2,double *featWeight)
{
	if (featWeight!=NULL)
	{
		switch (DistanceFunction)
		{
			case GML::Algorithm::Metrics::Manhattan:
				return GML::ML::VectorOp::ManhattanDistance(r1.Features,r2.Features,r1.FeatCount,featWeight);
			case GML::Algorithm::Metrics::Euclidean:
				return GML::ML::VectorOp::EuclideanDistance(r1.Features,r2.Features,r1.FeatCount,featWeight);
			case GML::Algorithm::Metrics::Euclidean_Square:
				return pow(GML::ML::VectorOp::EuclideanDistance(r1.Features,r2.Features,r1.FeatCount,featWeight),2);
			case GML::Algorithm::Metrics::Minkowski:
				return GML::ML::VectorOp::MinkowskiDistance(r1.Features,r2.Features,r1.FeatCount,DistancePower,featWeight);
			case GML::Algorithm::Metrics::ProcDifference:
				return GML::ML::VectorOp::ProcDistance(r1.Features,r2.Features,r1.FeatCount,featWeight);
		}
	} else {
		switch (DistanceFunction)
		{
			case GML::Algorithm::Metrics::Manhattan:
				return GML::ML::VectorOp::ManhattanDistance(r1.Features,r2.Features,r1.FeatCount);
			case GML::Algorithm::Metrics::Euclidean:
				return GML::ML::VectorOp::EuclideanDistance(r1.Features,r2.Features,r1.FeatCount);
			case GML::Algorithm::Metrics::Euclidean_Square:
				return GML::ML::VectorOp::EuclideanDistanceSquared(r1.Features,r2.Features,r1.FeatCount);
			case GML::Algorithm::Metrics::Minkowski:
				return GML::ML::VectorOp::MinkowskiDistance(r1.Features,r2.Features,r1.FeatCount,DistancePower);
			case GML::Algorithm::Metrics::ProcDifference:
				return GML::ML::VectorOp::ProcDistance(r1.Features,r2.Features,r1.FeatCount);
		}
	}
	// altfel verific functiile kernel
	switch (DistanceFunction)
	{
		case GML::Algorithm::Metrics::Binomial:
			return GML::ML::VectorOp::BinomialDistance(r1.Features,r2.Features,r1.FeatCount,DistancePower,DistanceBias);
		case GML::Algorithm::Metrics::Radial:
			return GML::ML::VectorOp::RadialDistance(r1.Features,r2.Features,r1.FeatCount,DistanceSigma);
		case GML::Algorithm::Metrics::Sigmoid:
			return GML::ML::VectorOp::SigmoidDistance(r1.Features,r2.Features,r1.FeatCount,DistancePower);
		case GML::Algorithm::Metrics::HyperbolicTangent:
			return GML::ML::VectorOp::HyperbolicTangentDistance(r1.Features,r2.Features,r1.FeatCount,DistanceK,DistanceBias);	
	}
	return 0;
}