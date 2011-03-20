#include "KNN.h"


bool	KNN_INFO::operator <(KNN_INFO &k1)
{
	return (bool)(Dist<k1.Dist);
}
bool	KNN_INFO::operator >(KNN_INFO &k1)
{
	return (bool)(Dist>k1.Dist);
}

KNN::KNN()
{
	ObjectName = "KNN";

	LinkPropertyToString("Connector",strConector,"","Conectorul la care sa se conecteze");
	LinkPropertyToString("Notifier",strNotificator,"","Notificatorul la care sa se conecteze");
	LinkPropertyToString("DataBase",strDB,"","Baza de date la care sa se conecteze");
	LinkPropertyToUInt32("K",k,1);
	LinkPropertyToUInt32("ThreadsCount",threadsCount,1);
	LinkPropertyToDouble("MemProcent",procMem,0.66,"O valoarea care identifica ....");
	LinkPropertyToUInt32("DistMethod",distMethod,DIST_EUCLIDIANA,"!!LIST:Euclidiana=0,ZuggyDist!!\nMetrica folosita");

	SetPropertyMetaData("Command","!!LIST:None=0,Test!!");
}

bool KNN::Init()
{
	if ((notif = GML::Builder::CreateNotifier(strNotificator.GetText()))==NULL)
		return false;
	if ((db = GML::Builder::CreateDataBase(strDB.GetText(),*notif))==NULL)
	{
		notif->Error("[%s] -> Unable to create Database (%s)",ObjectName,strDB.GetText());
		return false;
	}
	if (db->Connect()==false)
	{
		notif->Error("[%s] -> Unable to connesct to Database (%s)",ObjectName,strDB.GetText());
		return false;
	}
	if ((con = GML::Builder::CreateConnectors(strConector.GetText(),*notif,*db))==NULL)
	{
		notif->Error("[%s] -> Unable to create Conector (%s)",ObjectName,strConector.GetText());
		return false;
	}
	if ((procMem<=0) || (procMem>=1.0))
	{
		notif->Error("[%s] -> Procent memory should be between [0..1]",ObjectName);
		return false;
	}
	if (memIndex.Create(1+(UInt32)(procMem*con->GetRecordCount()))==false)
	{
		notif->Error("[%s] -> Unable to create memIndex",ObjectName);
		return false;
	}
	if (testIndex.Create(1+con->GetRecordCount()-(UInt32)(procMem*con->GetRecordCount()))==false)
	{
		notif->Error("[%s] -> Unable to create testIndex",ObjectName);
		return false;
	}
	if ((con->CreateMlRecord(ob1)==false) || (con->CreateMlRecord(ob2)==false))
	{
		notif->Error("[%s] -> Unable to create ob-urile",ObjectName);
		return false;
	}

	UInt32 tr;
	for (tr=0;tr<(procMem*con->GetRecordCount());tr++)
		memIndex.Push(tr);
	for (;tr<con->GetRecordCount();tr++)
		testIndex.Push(tr);

	notif->Info("Am %d inregistrari din care pastrez %lf %%",con->GetRecordCount(),procMem);
	return true;


}

void KNN::OnExecute()
{
	if (Command==Test)
	{
		notif->Info("Start testing ...");
		GML::Utils::GTVector<KNN_INFO>	list;
		UInt32							tr,gr,myIndex;
		KNN_INFO						element;
		GML::Utils::AlgorithmResult		res;


		res.Clear();
		for (tr=0;tr<testIndex.Len();tr++)
		{
			myIndex = testIndex.Get(tr);
			if (con->GetRecord(ob1,myIndex)==false)
				return;
			for (gr=0;gr<memIndex.Len();gr++)
			{
				element.indexElement = memIndex.Get(gr);
				if (con->GetRecord(ob2,myIndex)==false)
					return;
				element.Label = ob2.Label;
				element.Dist = GML::ML::VectorOp::PointToPointDistance(ob1.Features,ob2.Features,ob1.FeatCount);
				list.PushByRef(element);
				list.Sort(true);
				if (list.Len()==k+1)
					list.Delete(k);
			}
			// decizie
			gr=0;
			for (tr=0;tr<k;tr++)
				if (list[tr].Label==1)
					gr++;
			if (gr>k/2)
			{
				res.Update(ob1.Label==1,ob1.Label==1);				
			} else {
				res.Update(ob1.Label==1,ob1.Label==-1);				
			}
			
			res.Compute();
			notif->Notify(100,&res,sizeof(res));
		}
	}
}