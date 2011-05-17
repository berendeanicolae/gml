#include "KMeans.h"

#define COMPUTE_DISTANCES		1
#define ADJUST_CLUSTERS			2


bool ClustersList::Create(UInt32 clusterCount,UInt32 featCount)
{
	if ((Clusters = new Cluster[clusterCount])==NULL)
		return false;
	for (UInt32 tr=0;tr<clusterCount;tr++)
	{
		if ((Clusters[tr].Weight = new double[featCount])==NULL)
			return false;
		Clusters[tr].Count = featCount;
		Clusters[tr].ElementsCount = 0;
	}
	Count = clusterCount;
	return true;
}
//================================================================================


KMeans::KMeans()
{
	ObjectName = "KMeans";

	SetPropertyMetaData("Command","!!LIST:None=0,Train!!");
	
	LinkPropertyToUInt32("K",K,2,"");
	LinkPropertyToUInt32("MaxIterations",MaxIterations,100,"Maximum number of iterations");
	LinkPropertyToUInt32("InitialClusters",InitialClusters,INITIAL_RANDOM_VALUES,"!!LIST:RandomPos=0,RandomElements!!");
			
}


void KMeans::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand)
{
	switch (threadCommand)
	{
		case COMPUTE_DISTANCES:
			ComputeDistances(thData);
			return;
	};
}
bool KMeans::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	ClustersList	*cl = new ClustersList();
	if (cl->Create(K,con->GetFeatureCount())==false)
		return false;
	thData.Context = cl;
	return true;
}
bool KMeans::Init()
{
	if (InitConnections()==false)
		return false;
	if (K<2)
	{
		notif->Error("[%s] -> Invalid value for K (%d) ",ObjectName,K);
		return false;
	}
	if (Clusters.Create(K,con->GetFeatureCount())==false)
	{
		notif->Error("[%s] -> Unable to alloc %d clusters",ObjectName,K);
		return false;
	}

	if (InitThreads()==false)
		return false;
	if (SplitMLThreadDataRange(con->GetRecordCount())==false)
		return false;
	return true;
}
bool KMeans::ComputeDistances(GML::Algorithm::MLThreadData &thData)
{
	ClustersList	*cl = (ClustersList *)thData.Context;
	UInt32			tr,gr,size,index,featCount;
	double			minDist,dist;

	//if (thData.ThreadID==0)
	//	notif->StartProcent("[%s] -> Computing ... ",ObjectName);

	featCount = con->GetFeatureCount();
	size = thData.Range.Size();
	for (tr=0;tr<K;tr++)
	{
		cl->Clusters[tr].ElementsCount = 0;
		memset(cl->Clusters[tr].Weight,0,sizeof(double)*featCount);
	}

	for (tr=thData.Range.Start;tr<thData.Range.End;tr++)
	{
		if (con->GetRecord(thData.Record,tr)==false)
		{
			notif->Error("[%s] -> Unable to read record #%d",ObjectName,tr);
			return false;
		}
		
		index = 0;
		for (gr=0;gr<K;gr++)
		{
			dist = GML::ML::VectorOp::EuclideanDistanceSquared(thData.Record.Features,Clusters.Clusters[gr].Weight,featCount);
			if (gr==0)
			{
				minDist = dist;
				index = 0;
			} else {
				if (dist<minDist)
				{
					minDist = dist;
					index = gr;
				}
			}
		}
		cl->Clusters[index].ElementsCount++;
		for (gr=0;gr<featCount;gr++)
			cl->Clusters[index].Weight[gr]+=thData.Record.Features[gr];		

		//if (thData.ThreadID==0)
		//	notif->SetProcent(tr,size);
	}
	//if (thData.ThreadID==0)
	//	notif->EndProcent();
	return true;
}

bool KMeans::Train(UInt32 iteration)
{
	UInt32					tr,gr,hr;
	double					sum,div,newValue;
	ClustersList			*cl;
	bool					change=false;
	

	ExecuteParalelCommand(COMPUTE_DISTANCES);
	// adun datele
	
	tempStr.SetFormated("%5d => |",iteration);
	for (tr=0;tr<K;tr++)
	{
		div = 0;
		for (hr=0;hr<threadsCount;hr++)
		{
			cl = (ClustersList *)ThData[hr].Context;
			div += cl->Clusters[tr].ElementsCount;
		}
		tempStr.AddFormated("%5d|",(UInt32)div);
		for (gr=0;(gr<con->GetFeatureCount()) && (div!=0);gr++)
		{
			sum = 0;
			for (hr=0;hr<threadsCount;hr++)
			{
				cl = (ClustersList *)ThData[hr].Context;
				sum += cl->Clusters[tr].Weight[gr];				
			}
			newValue = sum/div;
			if (newValue!=Clusters.Clusters[tr].Weight[gr])
			{
				//notif->Info("Cluster:%d Feat:%d (Old=%lf,New=%lf)",tr,gr,Clusters.Clusters[tr].Weight[gr],newValue);
				Clusters.Clusters[tr].Weight[gr] = newValue;
				change = true;
				
			}
		}
	}
	notif->Info(tempStr.GetText());
	return change;
}
void KMeans::PerformTrain()
{
	for (UInt32 tr=0;tr<10;tr++)
		if (!Train(tr+1))
			break;
}
void KMeans::OnExecute()
{
	if (Command==1)	//Train
	{
		PerformTrain();
		return;
	}
	notif->Error("[%s] -> Unknown command ID: %d",ObjectName,Command);
}