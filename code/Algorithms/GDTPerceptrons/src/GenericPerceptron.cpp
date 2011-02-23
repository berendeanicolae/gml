#include "GenericPerceptron.h"


GenericPerceptron::GenericPerceptron()
{
	db = NULL;
	con = NULL;

	LinkPropertyToString("Name"					,Name					,"");
	LinkPropertyToString("DataBase"				,DataBase				,"");
	LinkPropertyToString("Conector"				,Conector				,"");
	LinkPropertyToString("Notifier"				,Notifier				,"");
	LinkPropertyToDouble("LearningRate"			,learningRate			,0.01);
	LinkPropertyToBool  ("UseWeight"			,useWeight				,false);
	LinkPropertyToUInt32("TestAfterIterations"	,testAfterIterations	,1);
	LinkPropertyToDouble("MinimAcc"				,minimAcc				,100.0);
	LinkPropertyToDouble("MinimSe"				,minimAcc				,100.1);
	LinkPropertyToDouble("MinimSp"				,minimAcc				,100.1);
	LinkPropertyToUInt32("MaxIterations"		,maxIterations			,10);
	LinkPropertyToString("WeightFileName"		,WeightFileName			,"");
	LinkPropertyToString("InitialWeight"		,InitialWeight			,"Zeros","!!LIST:Zeros,Random,FromFile!!");
}
bool	GenericPerceptron::Init()
{
	// creez obiectele:
	if ((notif = GML::Builder::CreateNotifyer(Notifier.GetText()))==NULL)
		return false;
	if ((db = GML::Builder::CreateDataBase(DataBase.GetText(),*notif))==NULL)
	{
		notif->Error("Unable to create Database (%s)",DataBase.GetText());
		return false;
	}
	if (db->Connect()==false)
	{
		notif->Error("Unable to connect to Database (%s)",DataBase.GetText());
		return false;
	}
	if ((con = GML::Builder::CreateConectors(Conector.GetText(),*notif,*db))==NULL)
	{
		notif->Error("Unable to create Conector (%s)",Conector.GetText());
		return false;
	}

	return true;
}
bool    GenericPerceptron::Train(PerceptronThreadData *ptd)
{
	UInt32	*ptrIndex = ptd->RecordIndexes;
	UInt32	count = ptd->RecordIndexesCount;
	UInt32	nrFeatures = con->GetFeatureCount();
	double	*w = ptd->Weight;
	double	*b = &ptd->b_Weight;
	double	error;

	if (ptd->Delta!=NULL)
	{
		w = ptd->Delta;
		b = &ptd->b_Delta;
	}

	while (count>0)
	{
		if (con->GetRecord(ptd->Record,*ptrIndex)==false)
		{
			notif->Error("(TRAIN)::Error reading record #%d from thread #%d",(*ptrIndex),ptd->ID);
			return false;
		}
		if (GML::ML::VectorOp::IsPerceptronTrained(ptd->Record.Features,w,nrFeatures,*b,ptd->Record.Label)==false)
		{
			if (useWeight)
				error = ptd->Record.Label * ptd->Record.Weight * learningRate;
			else
				error = ptd->Record.Label * learningRate;
			GML::ML::VectorOp::AdjustTwoStatePerceptronWeights(ptd->Record.Features,w,nrFeatures,error);
			(*b) += error;
		}
		count--;
		ptrIndex++;
	}
	if (ptd->Delta!=NULL)
	{
		ptd->b_Weight+=ptd->b_Delta;
		GML::ML::VectorOp::AddVectors(ptd->Weight,ptd->Delta,nrFeatures);
	}	
	return true;
}
bool	GenericPerceptron::Test(PerceptronThreadData *ptd)
{
	UInt32	*ptrIndex = ptd->RecordIndexes;
	UInt32	count = ptd->RecordIndexesCount;
	UInt32	nrFeatures = con->GetFeatureCount();
	double	*w = ptd->Weight;
	double	*b = &ptd->b_Weight;

	ptd->Res.Clear();

	while (count>0)
	{
		if (con->GetRecord(ptd->Record,*ptrIndex)==false)
		{
			notif->Error("(TEST)::Error reading record #%d from thread #%d",(*ptrIndex),ptd->ID);
			return false;
		}
		ptd->Res.Update(ptd->Record.Label==1,GML::ML::VectorOp::IsPerceptronTrained(ptd->Record.Features,w,nrFeatures,*b,ptd->Record.Label));
		count--;
		ptrIndex++;
	}
	ptd->Res.Compute();

	return true;
}