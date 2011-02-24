#include "GenericPerceptron.h"


GenericPerceptron::GenericPerceptron()
{
	db = NULL;
	con = NULL;

	batchPerceptron = false;

	LinkPropertyToString("Name"					,Name					,"");
	LinkPropertyToString("DataBase"				,DataBase				,"");
	LinkPropertyToString("Conector"				,Conector				,"");
	LinkPropertyToString("Notifier"				,Notifier				,"");
	LinkPropertyToDouble("LearningRate"			,learningRate			,0.01);
	LinkPropertyToBool  ("UseWeight"			,useWeight				,false);
	LinkPropertyToBool  ("UseB"					,useB					,true);
	LinkPropertyToUInt32("TestAfterIterations"	,testAfterIterations	,1);
	LinkPropertyToDouble("MinimAcc"				,minimAcc				,100.0);
	LinkPropertyToDouble("MinimSe"				,minimAcc				,100.1);
	LinkPropertyToDouble("MinimSp"				,minimAcc				,100.1);
	LinkPropertyToUInt32("MaxIterations"		,maxIterations			,10);
	LinkPropertyToString("WeightFileName"		,WeightFileName			,"");
	LinkPropertyToString("InitialWeight"		,InitialWeight			,"Zeros","!!LIST:Zeros,Random,FromFile!!");
	LinkPropertyToUInt32("ThreadsCount"			,threadsCount			,1);
}
bool	GenericPerceptron::SplitIndexes(PerceptronThreadData *ptd,UInt32 ptdElements,PerceptronThreadData *original)
{
	UInt32	tr,idx;
	UInt32	rap = (original->RecordIndexesCount/threadsCount)+1;
	UInt32	index[2] = {0,0};
	double	label;

	// aloc memorie pentru indexi
	for (tr=0;tr<threadsCount;tr++)
	{
		if ((ptd[tr].RecordIndexes = new UInt32[rap])==NULL)
		{
			notif->Error("Unable to allocate RecordIndexes[%d] vector !",rap);
			return false;
		}
		ptd[tr].RecordIndexesCount = 0;
	}
	// completez lista
	for (tr=0;tr<original->RecordIndexesCount;tr++)
	{
		if (con->GetRecordLabel(label,original->RecordIndexes[tr])==false)
		{
			notif->Error("Unable to read record with index #%d",original->RecordIndexes[tr]);
			return false;
		}
		if (label==1)
			idx= 1;
		else
			idx = 0;
		ptd[index[idx]].RecordIndexes[ptd[index[idx]].RecordIndexesCount++]=original->RecordIndexes[tr];
		index[idx]++;
		if (index[idx]>=ptdElements)
			index[idx]=0;
	}

	return true;
}
bool	GenericPerceptron::Init()
{
	UInt32		tr;

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
	if (batchPerceptron==false)
		threadsCount = 1;
	if (threadsCount<1)
		threadsCount = 1;

	// creez indexii pentru toate
	if (con->CreateMlRecord(FullData.Record)==false)
	{
		notif->Error("Unable to create MLRecord !");
		return false;
	}
	if ((FullData.Weight = new double[con->GetFeatureCount()])==NULL)
	{
		notif->Error("Unable to allocate weight vector !");
		return false;
	}
	memset(FullData.Weight,0,sizeof(double)*con->GetFeatureCount());
	FullData.b_Weight = 0;
	if (batchPerceptron)
	{
		if ((FullData.Delta = new double[con->GetFeatureCount()])==NULL)
		{
			notif->Error("Unable to allocate delta vector !");
			return false;
		}		
	} else {
		FullData.Delta = NULL;
	}
	FullData.ID = 0xFFFFFFFF;
	if ((FullData.RecordIndexes=new UInt32[con->GetRecordCount()])==NULL)
	{
		notif->Error("Unable to create RecordIndexes[%d] !",con->GetRecordCount());
		return false;
	}
	FullData.RecordIndexesCount = con->GetRecordCount();
	for (tr=0;tr<FullData.RecordIndexesCount;tr++)
		FullData.RecordIndexes[tr]=tr;


	if (threadsCount>1)
	{
		if ((ptData = new PerceptronThreadData[threadsCount])==NULL)
		{
			notif->Error("Unable to allocate PerceptronThreadData[%d]",threadsCount);
			return false;
		}
		for (tr=0;tr<threadsCount;tr++)
		{
			if (con->CreateMlRecord(ptData[tr].Record)==false)
			{
				notif->Error("Unable to create MLRecord !");
				return false;
			}
			if ((ptData[tr].Weight = new double[con->GetFeatureCount()])==NULL)
			{
				notif->Error("Unable to allocate weight vector !");
				return false;
			}
			if (batchPerceptron)
			{
				if ((ptData[tr].Delta = new double[con->GetFeatureCount()])==NULL)
				{
					notif->Error("Unable to allocate delta vector !");
					return false;
				}		
			} else {
				ptData[tr].Delta = NULL;
			}
			ptData[tr].ID = tr;		
		}
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