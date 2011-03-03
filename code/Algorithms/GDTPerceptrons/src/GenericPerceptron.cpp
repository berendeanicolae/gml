#include "GenericPerceptron.h"

void ThreadRedirectFunction(GML::Utils::IParalelUnit *paralel,void *context)
{
	GenericPerceptron *gp = (GenericPerceptron *)context;
	gp->OnRunThreadCommand(gp->ptData[paralel->GetID()],paralel->GetCodeID());
}
//====================================================================================================
PerceptronVector::PerceptronVector()
{
	Weight = NULL;
	Bias = NULL;
	Count = 0;
}
PerceptronVector::~PerceptronVector()
{
	Destroy();
}
void PerceptronVector::Destroy()
{
	if (Weight!=NULL)
		delete Weight;
	if (Bias)
		delete Bias;
	Weight = NULL;
	Bias = NULL;
	Count = 0;
}
bool PerceptronVector::Create(UInt32 count)
{
	Destroy();
	if ((Weight = new double[count])==NULL)
		return false;
	if ((Bias = new double)==NULL)
		return false;
	memset(Weight,0,sizeof(double)*count);
	(*Bias) = 0.0;
	Count = count;

	return true;
}
void PerceptronVector::Add(PerceptronVector &pv)
{
	GML::ML::VectorOp::AddVectors(Weight,pv.Weight,Count);
	(*Bias)+=(*pv.Bias);
}
bool PerceptronVector::Create(PerceptronVector &pv)
{
	Weight = pv.Weight;
	Bias = pv.Bias;
	Count = pv.Count;

	return true;
}
//====================================================================================================
PerceptronThreadData::PerceptronThreadData()
{
	RecordIndexes = NULL;
	RecordIndexesCount = 0;
	ExtraData = NULL;
}
//====================================================================================================
GenericPerceptron::GenericPerceptron()
{
	db = NULL;
	con = NULL;

	batchPerceptron = false;

	LinkPropertyToString("Name"						,Name					,"Perceptron");
	LinkPropertyToString("DataBase"					,DataBase				,"");
	LinkPropertyToString("Connector"				,Conector				,"");
	LinkPropertyToString("Notifier"					,Notifier				,"");
	LinkPropertyToDouble("LearningRate"				,learningRate			,0.01);
	LinkPropertyToBool  ("UseWeight"				,useWeight				,false);
	LinkPropertyToBool  ("UseBias"					,useB					,true);
	LinkPropertyToUInt32("SaveData"					,saveData				,SAVE_DATA_AT_FINISH,"!!LIST:None=0,AfterEachIteration,WhenAlgorithmEnds!!");
	LinkPropertyToUInt32("SaveBest"					,saveBest				,SAVE_BEST_NONE,"!!LIST:None=0,BestACC,BestSE,BestSP!!");
	LinkPropertyToUInt32("TestAfterIterations"		,testAfterIterations	,1);
	LinkPropertyToDouble("MinimAcc"					,minimAcc				,100.0);
	LinkPropertyToDouble("MinimSe"					,minimAcc				,100.1);
	LinkPropertyToDouble("MinimSp"					,minimAcc				,100.1);
	LinkPropertyToUInt32("MaxIterations"			,maxIterations			,10);
	LinkPropertyToString("WeightFileName"			,WeightFileName			,"");
	LinkPropertyToUInt32("InitialWeight"			,InitialWeight			,INITIAL_WEIGHT_ZERO,"!!LIST:Zeros=0,Random,FromFile!!");
	LinkPropertyToUInt32("ThreadsCount"				,threadsCount			,1);
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
bool	GenericPerceptron::Create(PerceptronThreadData &ptd,UInt32 id,PerceptronThreadData *original)
{
	if (con->CreateMlRecord(ptd.Record)==false)
	{
		notif->Error("[%s] -> Unable to create MLRecord !",ObjectName);
		return false;
	}
	if (original==NULL)
	{
		if (ptd.Primary.Create(con->GetFeatureCount())==false)
		{
			notif->Error("[%s] -> Unable to allocate prymary vector !",ObjectName);
			return false;
		}		
	} else {
		if (ptd.Primary.Create(original->Primary)==false)
		{
			notif->Error("[%s] -> Unable to allocate prymary vector !",ObjectName);
			return false;
		}	
	}
	if (batchPerceptron)
	{
		if (ptd.Delta.Create(con->GetFeatureCount())==false)
		{
			notif->Error("[%s] -> Unable to allocate delta vector !",ObjectName);
			return false;
		}		
	} else {
		ptd.Delta.Destroy();
	}
	ptd.RecordIndexes = NULL;
	ptd.RecordIndexesCount = 0;
	ptd.Res.Clear();
	ptd.ID = id;
	return true;	
}
bool	GenericPerceptron::UpdateBest(PerceptronThreadData &ptd)
{
	switch (saveBest)
	{
		case SAVE_BEST_NONE:
			return false;
		case SAVE_BEST_ACC:
			if (ptd.Res.acc<BestData.Res.acc)
				return false;
			break;
		case SAVE_BEST_SE:
			if (ptd.Res.se<BestData.Res.se)
				return false;
			break;
		case SAVE_BEST_SP:
			if (ptd.Res.sp<BestData.Res.sp)
				return false;
			break;
		default:
			notif->Error("UpdateBest => false (code = %d) ",saveBest);
			return false;
	}
	// copii datele
	memcpy(BestData.Primary.Weight,ptd.Primary.Weight,con->GetFeatureCount()*sizeof(double));
	BestData.Primary.Bias = ptd.Primary.Bias;
	BestData.Res.Copy(&ptd.Res);

	return true;
}
bool	GenericPerceptron::Save(PerceptronThreadData &ptd,char *fileName)
{
	GML::Utils::AttributeList	tempAttr;

	tempAttr.Clear();
	if (tempAttr.AddDouble("b",*ptd.Primary.Bias,"b value form line equation Sum(xi*wi)+b")==false)
	{
		notif->Error("Unable to populate AttributeList for saving ...");
		return false;
	}
	if (tempAttr.AddAttribute("Weight",ptd.Primary.Weight,GML::Utils::AttributeList::DOUBLE,ptd.Primary.Count,"w value form line equation Sum(xi*wi)+b")==false)
	{
		notif->Error("Unable to populate AttributeList for saving ...");
		return false;
	}
	if (tempAttr.AddDouble("acc",ptd.Res.acc)==false)
	{
		notif->Error("Unable to populate AttributeList for saving ...");
		return false;
	}
	if (tempAttr.AddDouble("se",ptd.Res.se)==false)
	{
		notif->Error("Unable to populate AttributeList for saving ...");
		return false;
	}
	if (tempAttr.AddDouble("sp",ptd.Res.sp)==false)
	{
		notif->Error("Unable to populate AttributeList for saving ...");
		return false;
	}

	if (tempAttr.Save(fileName)==false)
	{
		notif->Error("Unable to save data to %s",fileName);
		return false;
	}
	return true;
}
bool	GenericPerceptron::Load(PerceptronThreadData &ptd,char *fileName)
{
	GML::Utils::AttributeList	tempAttr;

	if (tempAttr.Load(fileName)==false)
	{
		notif->Error("Unable to load : %s",fileName);
		return false;
	}
	if (tempAttr.UpdateDouble("b",*ptd.Primary.Bias,true)==false)
	{
		notif->Error("Unable to update 'b' value from %s",fileName);
		return false;
	}
	if (tempAttr.Update("Weight",ptd.Primary.Weight,sizeof(double)*ptd.Primary.Count)==false)
	{
		notif->Error("Unable to update 'Weight' value from %s",fileName);
		return false;
	}
	return true;
}
bool	GenericPerceptron::InitWeight(PerceptronThreadData &ptd)
{
	UInt32	tr;

	if (ptd.Primary.Weight==NULL)
		return true;
	switch (InitialWeight)
	{
		case INITIAL_WEIGHT_ZERO:
			memset(ptd.Primary.Weight,0,sizeof(double)*ptd.Primary.Count);
			(*ptd.Primary.Bias) = 0.0;
			break;
		case INITIAL_WEIGHT_RANDOM:
			for (tr=0;tr<ptd.Primary.Count;tr++)
				ptd.Primary.Weight[tr]=((double)((rand()%201)-100))/100.0;
			(*ptd.Primary.Bias) = ((double)((rand()%201)-100))/100.0;
			break;
		case INITIAL_WEIGHT_FROMFILE:
			if (Load(ptd,WeightFileName.GetText())==false)
				return false;
			break;
		default:
			notif->Error("Unknwo initial seright method : %d ",InitialWeight);
			return false;
	}
	return true;
}
bool	GenericPerceptron::Init()
{
	UInt32		tr;

	// creez obiectele:
	if ((notif = GML::Builder::CreateNotifier(Notifier.GetText()))==NULL)
		return false;
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
	if (batchPerceptron==false)
		threadsCount = 1;
	if (threadsCount<1)
		threadsCount = 1;

	// creez indexii pentru toate
	if (Create(FullData,0xFFFFFFFF)==false)
		return false;
	if (InitWeight(FullData)==false)
		return false;
	if (Create(BestData,0xFFFFFFFF)==false)
		return false;
	
	if ((FullData.RecordIndexes = new UInt32[con->GetRecordCount()])==NULL)
	{
		notif->Error("[%s] -> Unable to create RecordIndexes[%d] !",ObjectName,con->GetRecordCount());
		return false;
	}
	FullData.RecordIndexesCount = con->GetRecordCount();
	for (tr=0;tr<FullData.RecordIndexesCount;tr++)
		FullData.RecordIndexes[tr]=tr;


	if (threadsCount>1)
	{
		if ((ptData = new PerceptronThreadData[threadsCount])==NULL)
		{
			notif->Error("[%s] -> Unable to allocate PerceptronThreadData[%d]",ObjectName,threadsCount);
			return false;
		}
		for (tr=0;tr<threadsCount;tr++)
		{
			if (Create(ptData[tr],tr,&FullData)==false)
				return false;
		}
		if (SplitIndexes(ptData,threadsCount,&FullData)==false)
			return false;
		
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
		}
	}

	return OnInit();
}
bool    GenericPerceptron::Train(PerceptronThreadData *ptd,bool clearDelta,bool addDeltaToPrimary)
{
	UInt32	*ptrIndex = ptd->RecordIndexes;
	UInt32	count = ptd->RecordIndexesCount;
	UInt32	nrFeatures = con->GetFeatureCount();
	double	*w = ptd->Primary.Weight;
	double	*b = ptd->Primary.Bias;
	double	error;

	if (ptd->Delta.Weight!=NULL)
	{
		w = ptd->Delta.Weight;
		b = ptd->Delta.Bias;
		if (clearDelta)
		{
			memset(w,0,sizeof(double)*ptd->Delta.Count);
			(*b)=0;
		}
	}
	if (!useB)
		(*b)=0;

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
			if (useB)
				(*b) += error;
		}
		count--;
		ptrIndex++;
	}
	if ((ptd->Delta.Weight!=NULL) && (addDeltaToPrimary))
		ptd->Primary.Add(ptd->Delta);
	return true;
}
bool	GenericPerceptron::Test(PerceptronThreadData *ptd)
{
	UInt32	*ptrIndex = ptd->RecordIndexes;
	UInt32	count = ptd->RecordIndexesCount;
	UInt32	nrFeatures = con->GetFeatureCount();
	double	*w = ptd->Primary.Weight;
	double	*b = ptd->Primary.Bias;
	
	if (!useB)
		(*b)=0;

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
bool	GenericPerceptron::PerformTrain()
{
	UInt32					it;
	GML::Utils::GString		saveNM;

	FullData.Res.time.Start();

	for (it=0;it<maxIterations;it++)
	{
		if (PerformTrainIteration()==false)
		{
			notif->Error("Error on training iteration ...");
			return false;
		}
		if ((it % testAfterIterations)==0)
		{
			if (PerformTestIteration()==false)
			{
				notif->Error("Error on test iteration ...");
				return false;
			}
			if (OnUpdateBestData())
			{
				switch (saveBest)
				{
					case SAVE_BEST_ACC: 
						saveNM.SetFormated("%s_best_acc.txt",Name.GetText()); 
						break;
					case SAVE_BEST_SE: 
						saveNM.SetFormated("%s_best_se.txt",Name.GetText()); 
						break;
					case SAVE_BEST_SP: 
						saveNM.SetFormated("%s_best_sp.txt",Name.GetText()); 
						break;
					default:
						saveNM.SetFormated("%s_best.txt",Name.GetText());
						break;
				}				
				OnSaveData(saveNM.GetText());
			}
			if (saveData==SAVE_DATA_AFTER_EACH_ITERATION)
			{
				saveNM.SetFormated("%s_it_%d.txt",Name.GetText(),it+1);
				OnSaveData(saveNM.GetText());
			}
			FullData.Res.time.Start();
		}
	}
	if (saveData==SAVE_DATA_AT_FINISH)
	{
		saveNM.SetFormated("%s_final.txt",Name.GetText());
		OnSaveData(saveNM.GetText());
	}
	return true;
}
bool	GenericPerceptron::PerformTest()
{
	GML::Utils::GString		saveNM;
	if (PerformTestIteration()==false)
	{
		notif->Error("Error on test iteration ...");
		return false;
	}		
	return true;
}
void	GenericPerceptron::OnExecute(char *command)
{
	if (GML::Utils::GString::Equals(command,"train",true))
	{
		PerformTrain();
		return;
	}
	if (GML::Utils::GString::Equals(command,"test",true))
	{
		PerformTest();
		return;
	}
	notif->Error("Unkwnown command: %s",command);
}
bool    GenericPerceptron::ExecuteParalelCommand(UInt32 command)
{
	UInt32	tr;

	// executie
	for (tr=0;tr<threadsCount;tr++)
		if (tpu[tr].Execute(command)==false)
		{
			notif->Error("Error on runnig thread #%d",tr);
			return false;
		}
	// asteptare
	for (tr=0;tr<threadsCount;tr++)
		if (tpu[tr].WaitToFinish()==false)
		{
			notif->Error("WaitToFinish failed on thread #%d",tr);
			return false;
		}
	// all ok
	return true;
}
bool	GenericPerceptron::OnInit()
{
	return true;
}