#include "SimplePerceptronAlgorithm.h"

SimplePerceptronAlgorithm::SimplePerceptronAlgorithm()
{
	db = NULL;
	con = NULL;
	notif = NULL;
}
bool	SimplePerceptronAlgorithm::SetConfiguration(GML::Utils::AttributeList &config)
{
	if (config.UpdateString("DbName",dbName)==false)
		return false;
	if (config.UpdateString("DbConnString",conString)==false)
		return false;
	if (config.UpdateString("Conector",conectorString)==false)
		return false;
	if (config.UpdateString("Notifyer",notifString)==false)
		return false;
	if (config.Update("LearningRate",&learningRate,sizeof(double))==false)
		return false;
	if (config.Update("MaxIteratii",&maxIteratii,sizeof(UInt32))==false)
		return false;

	return true;
}
bool	SimplePerceptronAlgorithm::GetConfiguration(GML::Utils::AttributeList &config)
{
	return false;
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
	return true;
}
void	SimplePerceptronAlgorithm::Train()
{
	UInt32	tr;

	for (tr=0;tr<con->GetRecordCount();tr++)
	{
		if (con->GetRecord(rec,tr)==false)
		{
			notif->Error("Unable to read record #%d",tr);
			return;
		}
		if (GML::ML::VectorOp::IsPerceptronTrained(rec.Features,weight,rec.FeatCount,b,rec.Label)==false)
		{
			GML::ML::VectorOp::AdjustTwoStatePerceptronWeights(rec.Features,weight,rec.FeatCount,rec.Label*learningRate);
			b+=learningRate*rec.Label;
		}
	}
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
void	SimplePerceptronAlgorithm::Execute(UInt32 command)
{
	notif->Info("Starting Algorithm (Execute) => Command = %d",command);
	notif->Info("DB: Records = %d,Features = %d",con->GetRecordCount(),con->GetFeatureCount());
	memset(weight,0,sizeof(double)*con->GetFeatureCount());
	b=0;
	for (UInt32 tr = 0;tr<maxIteratii;tr++)
	{
		//notif->Info("Train (iteration = %d)",tr);
		Train();
		//notif->Info("Test  (iteration = %d)",tr);
		Test();
		//notif->Info("W=[%.3lf,%.3lf] b=%.3lf",weight[0],weight[1],b);
		//_asm nop;
	}
}