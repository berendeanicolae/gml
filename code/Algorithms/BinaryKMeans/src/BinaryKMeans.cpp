#include "BinaryKMeans.h"

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
		Clusters[tr].PositiveCount = 0;
		Clusters[tr].NegativeCount = 0;
	}
	Count = clusterCount;
	return true;
}
bool ClustersList::Save(char *fName)
{
	GML::Utils::GString		tmp;
	GML::Utils::File		f;

	if (f.Create(fName)==false)
		return false;
	for (UInt32 tr=0;tr<Count;tr++)
	{
		if (tmp.AddFormated("%lf:",Clusters[tr].Label)==false)
			return false;
		for (UInt32 gr=0;gr<Clusters[tr].Count;gr++)
			if (tmp.AddFormated("%lf,",Clusters[tr].Weight[gr])==false)
				return false;
		if (tmp.Add("\n")==false)
			return false;
	}
	if (f.Write(tmp.GetText(),tmp.Len())==false)
		return false;
	f.Close();
	return true;
}
bool Cluster::Save(char *fName)
{
	GML::Utils::AttributeList	a;
	
	if (a.AddAttribute("Weight",Weight,GML::Utils::AttributeList::DOUBLE,Count)==false)
		return false;
	if (a.AddUInt32("PositiveCount",PositiveCount)==false)
		return false;
	if (a.AddUInt32("NegativeCount",NegativeCount)==false)
		return false;
	if (a.AddDouble("Label",Label)==false)
		return false;
	if (a.Save(fName)==false)
		return false;
	return true;
}
bool Cluster::Load(char *fName)
{
	GML::Utils::AttributeList	a;

	if (a.Load(fName)==false)
		return false;
	if (a.Update("Weight",Weight,sizeof(double)*Count)==false)
		return false;
	if (a.UpdateUInt32("PositiveCount",PositiveCount)==false)
		return false;
	if (a.UpdateUInt32("NegativeCount",NegativeCount)==false)
		return false;
	a.UpdateDouble("Label",Label,true);
	return true;
}
//================================================================================


BinaryKMeans::BinaryKMeans()
{
	ObjectName = "BinaryKMeans";

	SetPropertyMetaData("Command","!!LIST:None=0,Train!!");
	
	LinkPropertyToUInt32("K",K,2,"");
	LinkPropertyToUInt32("MaxIterations",MaxIterations,100,"Maximum number of iterations");
	LinkPropertyToUInt32("InitialClusters",InitialClusters,INITIAL_RANDOM_VALUES,"!!LIST:RandomPos=0,RandomElements!!");
	LinkPropertyToDouble("MinRandomValue",minRandomValue,-1);
	LinkPropertyToDouble("MaxRandomValue",maxRandomValue,1);
	LinkPropertyToString("ResultFileName",ResultFileName,"","The name of the file where the results will be written");
	LinkPropertyToUInt32("ResultFileType",ResultFileType,SAVE_METHOD_NORMAL,"!!LIST:Normal=0,CSV,Both!!");
	LinkPropertyToBool  ("SaveAfterEachIteration",SaveAfterEachIteration,false,"Specify if data should be saved after each iteration");
}

bool BinaryKMeans::SaveData(char *fName)
{
	UInt32					gr;
	GML::Utils::GString		tmp;

	while (true)
	{
		if ((ResultFileType==SAVE_METHOD_NORMAL) || (ResultFileType==SAVE_METHOD_BOTH))
		{
			for (gr=0;gr<Clusters.Count;gr++)
			{
				
				if (tmp.SetFormated("%s_CL_%d.txt",fName,gr)==false)
					break;				
				if (Clusters.Clusters[gr].Save(tmp.GetText())==false)
					break;				
			}
			if (gr!=Clusters.Count)
				break;
		}
		if ((ResultFileType==SAVE_METHOD_CSV) || (ResultFileType==SAVE_METHOD_BOTH))
		{
			if (tmp.SetFormated("%s.csv",fName)==false)
				break;
			if (Clusters.Save(tmp.GetText())==false)
				break;
		}
		return true;
	}
	notif->Error("[%s] -> Unable to save data to: %s",ObjectName,fName);
	return false;
}
void BinaryKMeans::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand)
{
	switch (threadCommand)
	{
		case COMPUTE_DISTANCES:
			ComputeDistances(thData);
			return;
	};
}
bool BinaryKMeans::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	ClustersList	*cl = new ClustersList();
	if (cl->Create(K,con->GetFeatureCount())==false)
		return false;
	thData.Context = cl;
	return true;
}
bool BinaryKMeans::InitRandomValues()
{
	double	value;
	double	dif;

	if (minRandomValue>=maxRandomValue)
	{
		notif->Error("[%s] -> Invalid 'MinRandomValue' or 'MaxRandomValue' properties",ObjectName);
		return false;
	}
	dif = maxRandomValue - minRandomValue;
	srand(GetTickCount());
	for (UInt32 tr=0;tr<K;tr++)
	{
		for (UInt32 gr=0;gr<con->GetFeatureCount();gr++)
		{
			value = (double)(rand() % 10000000);
			value = ((value * dif)/10000000.0) + minRandomValue;
			Clusters.Clusters[tr].Weight[gr] = value;
			Clusters.Clusters[tr].Label = 0.0;
		}
	}
	return true;
}
bool BinaryKMeans::InitRandomElements()
{
	UInt32	index;
	srand(GetTickCount());
	for (UInt32 tr=0;tr<K;tr++)
	{
		index = rand() % (con->GetRecordCount());
		if (con->GetRecord(MainRecord,index)==false)
		{
			notif->Error("[%s] -> Unable to read record #%d",ObjectName,index);
			return false;
		}
		memcpy(Clusters.Clusters[tr].Weight,MainRecord.Features,sizeof(double)*con->GetFeatureCount());
		Clusters.Clusters[tr].Label = MainRecord.Label;
		notif->Info("[%s] -> Cluster #%d initialized as element %d (Labeled = %lf)",ObjectName,tr,index,MainRecord.Label);
	}
	return true;
}
bool BinaryKMeans::Init()
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
	if (con->CreateMlRecord(MainRecord)==false)
	{
		notif->Error("[%s] -> Unable to create MainRecord",ObjectName);
		return false;
	}
	switch (InitialClusters)
	{
		case INITIAL_RANDOM_VALUES:
			if (InitRandomValues()==false)
				return false;
			break;
		case INITIAL_RANDOM_ELEMENTS:
			if (InitRandomElements()==false)
				return false;
			break;
		default:
			notif->Error("[%s] -> Unknown 'InitialCluster' property value (%d)",ObjectName,InitialClusters);
			return false;
	}
	if (InitThreads()==false)
		return false;
	if (SplitMLThreadDataRange(con->GetRecordCount())==false)
		return false;
	return true;
}
bool BinaryKMeans::ComputeDistances(GML::Algorithm::MLThreadData &thData)
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
		cl->Clusters[tr].PositiveCount = cl->Clusters[tr].NegativeCount = 0;
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
		if (thData.Record.Label == 1)
			cl->Clusters[index].PositiveCount++;
		else
			cl->Clusters[index].NegativeCount++;

		for (gr=0;gr<featCount;gr++)
			cl->Clusters[index].Weight[gr]+=thData.Record.Features[gr];		

		//if (thData.ThreadID==0)
		//	notif->SetProcent(tr,size);
	}
	//if (thData.ThreadID==0)
	//	notif->EndProcent();
	return true;
}

bool BinaryKMeans::Train(UInt32 iteration)
{
	UInt32					tr,gr,hr,pCount,nCount;
	double					sum,div,newValue;
	ClustersList			*cl;
	bool					change=false;
	

	ExecuteParalelCommand(COMPUTE_DISTANCES);
	// adun datele
	
	tempStr.SetFormated("%5d => |",iteration);
	for (tr=0;tr<K;tr++)
	{
		div = 0;
		pCount = nCount = 0;
		for (hr=0;hr<threadsCount;hr++)
		{
			cl = (ClustersList *)ThData[hr].Context;
			pCount += cl->Clusters[tr].PositiveCount;
			nCount += cl->Clusters[tr].NegativeCount;
		}
		div = pCount+nCount;
		//tempStr.AddFormated("P:%5d(%3.2lf) N:%5d(%3.2lf)|",pCount,(double)(pCount*100/div),nCount,(double)(nCount*100/div));
		//if (pCount>=nCount)
		//	tempStr.AddFormated("P:%5d(%3.2lf)|",pCount+nCount,(double)(pCount*100/div));
		//else
		//	tempStr.AddFormated("N:%5d(%3.2lf)|",pCount+nCount,(double)(nCount*100/div));

		if (pCount>=nCount)
			tempStr.AddFormated("P:%.2lf|",(pCount*100/div));
		else
			tempStr.AddFormated("N:%.2lf|",(nCount*100/div));


		Clusters.Clusters[tr].PositiveCount = pCount;
		Clusters.Clusters[tr].NegativeCount = nCount;
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
	if (SaveAfterEachIteration)
	{
		tempStr.SetFormated("%s_it_%d",ResultFileName.GetText(),iteration);
		SaveData(tempStr.GetText());
	}
	return change;
}
void BinaryKMeans::PerformTrain()
{
	for (UInt32 tr=0;tr<10;tr++)
		if (!Train(tr+1))
			break;
	tempStr.SetFormated("%s_final",ResultFileName.GetText());
	SaveData(tempStr.GetText());
}
void BinaryKMeans::OnExecute()
{
	if (Command==1)	//Train
	{
		PerformTrain();
		return;
	}
	notif->Error("[%s] -> Unknown command ID: %d",ObjectName,Command);
}