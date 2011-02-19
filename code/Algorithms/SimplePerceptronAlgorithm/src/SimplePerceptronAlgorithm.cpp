#include "SimplePerceptronAlgorithm.h"

SimplePerceptronAlgorithm::SimplePerceptronAlgorithm()
{
	db = NULL;
	con = NULL;

	LinkPropertyToString("DbName"				,dbName			,"");
	LinkPropertyToString("DbConnString"		,conString		,"");
	LinkPropertyToString("Conector"			,conectorString	,"");
	LinkPropertyToString("Notifyer"			,notifString	,"");
	LinkPropertyToDouble("LearningRate"		,learningRate	,0.01);
	LinkPropertyToUInt32("MaxIteratii"			,maxIteratii	,100);
}
bool	SimplePerceptronAlgorithm::Init()
{
	// creez obiectele:
	if ((notif = GML::Builder::CreateNotifyer(notifString.GetText()))==NULL)
		return false;
	if ((db = GML::Builder::CreateDataBase(dbName.GetText(),*notif,conString.GetText()))==NULL)
	{
		notif->Error("Unable to create Database (%s,%s)",dbName.GetText(),conString.GetText());
		return false;
	}
	if (db->Connect()==false)
	{
		notif->Error("Unable to connect to Database (%s,%s)",dbName.GetText(),conString.GetText());
		return false;
	}
	if ((con = GML::Builder::CreateConectors(conectorString.GetText(),*notif,*db))==NULL)
	{
		notif->Error("Unable to create Conector (%s)",conectorString.GetText());
		return false;
	}
	if (con->CreateMlRecord(rec)==false)
	{
		notif->Error("Unable to create ML Record !");
		return false;
	}
	if ((weight = new double[rec.FeatCount])==NULL)
	{
		notif->Error("Unable to allocate %d features !",rec.FeatCount);
		return false;
	}
	if ((delta = new double[rec.FeatCount])==NULL)
	{
		notif->Error("Unable to allocate %d features (delta)!",rec.FeatCount);
		return false;
	}
	return true;
}
void	SimplePerceptronAlgorithm::Train()
{
	UInt32	tr;


	memset(delta,0,sizeof(double)*con->GetFeatureCount());
	b_delta = 0;

	for (tr=0;tr<con->GetRecordCount();tr++)
	{
		if (con->GetRecord(rec,tr)==false)
		{
			notif->Error("Unable to read record #%d",tr);
			return;
		}
		//bool is_tr = GML::ML::VectorOp::IsPerceptronTrained(rec.Features,weight,rec.FeatCount,b,rec.Label);
		//notif->Info("(Coord=%.1lf,%.1lf), W=(%2.3lf,%2.3lf) b=%2.3lf Label=%.1lf => %d",rec.Features[0],rec.Features[1],weight[0],weight[1],b,rec.Label,is_tr);		
		if (GML::ML::VectorOp::IsPerceptronTrained(rec.Features,weight,rec.FeatCount,b,rec.Label)==false)
		{
			//GML::ML::VectorOp::AdjustTwoStatePerceptronWeights(rec.Features,delta,rec.FeatCount,rec.Label*learningRate);
			//b_delta+=learningRate*rec.Label;
			GML::ML::VectorOp::AdjustTwoStatePerceptronWeights(rec.Features,weight,rec.FeatCount,rec.Label*learningRate);
			b+=learningRate*rec.Label;
		}
	}
	//notif->Info("W=(%2.3lf,%2.3lf) b=%2.3lf",weight[0],weight[1],b);		
	//notif->Info("(Coord=%.1lf,%.1lf), W=(%2.3lf,%2.3lf) b=%2.3lf Label=%.1lf",rec.Features[0],rec.Features[1],weight[0],weight[1],b,rec.Label);
	//GML::ML::VectorOp::AddVectors(weight,delta,con->GetFeatureCount());
	//b+=b_delta;
}
void	SimplePerceptronAlgorithm::Test()
{
	UInt32							tr;
	GML::Utils::AlgorithmResult		res;

	res.Clear();
	for (tr=0;tr<con->GetRecordCount();tr++)
	{
		if (con->GetRecord(rec,tr)==false)
		{
			notif->Error("Unable to read record #%d",tr);
			return;
		}
		res.Update(rec.Label==1,GML::ML::VectorOp::IsPerceptronTrained(rec.Features,weight,rec.FeatCount,b,rec.Label));
	}
	res.Compute();
	notif->Notify(100,&res,sizeof(res));
}
void	SimplePerceptronAlgorithm::OnExecute(char* command)
{
	notif->Info("Executing command : %s",command);
	if (GML::Utils::GString::Equals(command,"train",true))
	{
		notif->Info("DB: Records = %d,Features = %d",con->GetRecordCount(),con->GetFeatureCount());
		memset(&weight[0],0,sizeof(double)*con->GetFeatureCount());
		b=0;
		for (UInt32 tr = 0;tr<maxIteratii;tr++)
		{
			Train();
			Test();
		}
		return;
	}
	if (GML::Utils::GString::Equals(command,"test",true))
	{
		Test();
		return;
	}
	notif->Info("Unknown command : %s",command);
}