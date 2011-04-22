#include "GenericPerceptron.h"

UInt32 CountActiveFeatures(double *feat,UInt32 nrElements)
{
	UInt32	sum = 0;
	while (nrElements>0)
	{
		if ((*feat)!=0)
			sum++;
		feat++;
		nrElements--;
	}
	return sum;
}
//====================================================================================================
PerceptronVector::PerceptronVector()
{
	Weight = NULL;
	Bias = 0;
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
	Weight = NULL;
	Bias = 0;
	Count = 0;
}
bool PerceptronVector::Create(UInt32 count)
{
	Destroy();
	if ((Weight = new double[count])==NULL)
		return false;
	memset(Weight,0,sizeof(double)*count);
	Bias = 0.0;
	Count = count;

	return true;
}
void PerceptronVector::Add(PerceptronVector &pv)
{
	GML::ML::VectorOp::AddVectors(Weight,pv.Weight,Count);
	Bias+=pv.Bias;
}

bool PerceptronVector::Copy(PerceptronVector &pv)
{
	if (Count!=pv.Count)
		if (Create(pv.Count))
			return false;
	Bias = pv.Bias;
	memcpy(Weight,pv.Weight,sizeof(double)*pv.Count);

	return true;
}
void PerceptronVector::ResetValues()
{
	if (Weight)
		memset(Weight,0,sizeof(double)*Count);
	Bias = 0.0;
}

//====================================================================================================
GenericPerceptron::GenericPerceptron()
{
	featWeight = NULL;

	SetPropertyMetaData("Command","!!LIST:None=0,Train,Test!!");

	LinkPropertyToString("Name"						,Name					,"Perceptron");
	LinkPropertyToDouble("LearningRate"				,learningRate			,0.01);
	LinkPropertyToDouble("PozitiveLearningRate"		,pozitiveLearningRate	,0.01);
	LinkPropertyToDouble("NegativeLearningRate"		,negativeLearningRate	,0.01);
	LinkPropertyToBool  ("UseBias"					,useB					,true);
	LinkPropertyToUInt32("SaveData"					,saveData				,SAVE_DATA_AT_FINISH,"!!LIST:None=0,AfterEachIteration,WhenAlgorithmEnds!!");
	LinkPropertyToUInt32("SaveBest"					,saveBest				,SAVE_BEST_NONE,"!!LIST:None=0,BestACC,BestSE,BestSP!!");
	LinkPropertyToUInt32("TestAfterIterations"		,testAfterIterations	,1);
	LinkPropertyToDouble("MinimAcc"					,minimAcc				,100.0);
	LinkPropertyToDouble("MinimSe"					,minimSe				,100.1);
	LinkPropertyToDouble("MinimSp"					,minimSp				,100.1);
	LinkPropertyToUInt32("MaxIterations"			,maxIterations			,10);
	LinkPropertyToString("WeightFileName"			,WeightFileName			,"");
	LinkPropertyToUInt32("InitialWeight"			,InitialWeight			,INITIAL_WEIGHT_ZERO,"!!LIST:Zeros=0,Random,Statistics,Relevant,FromFile!!");
	LinkPropertyToUInt32("AdjustWeightMode"			,adjustWeightMode		,ADJUST_WEIGHT_LEARNING_RATE,"!!LIST:UseLearningRate=0,UseWeight,UseLeastMeanSquare,UseSplitLearningRate,UseSplitLeastMeanSquare,UseFeaturesWeight,UsePozitiveNegativeLearningRate!!");
	LinkPropertyToString("FeaturesWeightFile"		,FeaturesWeightFile		,"");
}

bool	GenericPerceptron::CreateIndexes()
{
	UInt32	tr,nrRec;

	nrRec = con->GetRecordCount();
	//nrRec = 50;

	notif->Info("[%s] -> Creating %d indexes ",ObjectName,nrRec);
	if (RecordIndexes.Create(nrRec)==false)
	{
		notif->Error("[%s] -> Unable to create Indexes[%d] !",ObjectName,nrRec);
		return false;
	}
	for (tr=0;tr<nrRec;tr++)
	{
		if (RecordIndexes.Push(tr)==false)
		{
			notif->Error("[%s] -> Unable to add Indexes %d !",ObjectName,tr);
			return false;
		}
	}
	return true;
}

bool	GenericPerceptron::Save(PerceptronVector &pv,char *fileName,GML::Utils::AlgorithmResult *result)
{
	GML::Utils::AttributeList	tempAttr;

	tempAttr.Clear();
	if (tempAttr.AddDouble("Bias",pv.Bias,"b value form line equation Sum(xi*wi)+b")==false)
	{
		notif->Error("Unable to populate AttributeList for saving ...");
		return false;
	}
	if (tempAttr.AddAttribute("Weight",pv.Weight,GML::Utils::AttributeList::DOUBLE,pv.Count,"w value form line equation Sum(xi*wi)+b")==false)
	{
		notif->Error("Unable to populate AttributeList for saving ...");
		return false;
	}
	if (result!=NULL)
	{
		if (tempAttr.AddDouble("acc",result->acc)==false)
		{
			notif->Error("[%s] -> Unable to populate AttributeList with 'acc' for saving ...",ObjectName);
			return false;
		}
		if (tempAttr.AddDouble("se",result->se)==false)
		{
			notif->Error("[%s] -> Unable to populate AttributeList with 'se' for saving ...",ObjectName);
			return false;
		}
		if (tempAttr.AddDouble("sp",result->sp)==false)
		{
			notif->Error("[%s] -> Unable to populate AttributeList with 'sp' for saving ...",ObjectName);
			return false;
		}
	}
	if (tempAttr.Save(fileName)==false)
	{
		notif->Error("[%s] -> Unable to save data to %s",ObjectName,fileName);
		return false;
	}
	return true;
}
bool	GenericPerceptron::Load(PerceptronVector &pv,char *fileName)
{
	GML::Utils::AttributeList	tempAttr;

	if (tempAttr.Load(fileName)==false)
	{
		notif->Error("Unable to load : %s",fileName);
		return false;
	}
	if (tempAttr.UpdateDouble("Bias",pv.Bias,true)==false)
	{
		notif->Error("Unable to update 'b' value from %s",fileName);
		return false;
	}
	if (tempAttr.Update("Weight",pv.Weight,sizeof(double)*pv.Count)==false)
	{
		notif->Error("Unable to update 'Weight' value from %s",fileName);
		return false;
	}
	return true;
}
bool	GenericPerceptron::InitWeight(PerceptronVector &pv)
{
	UInt32	tr,recCount,gr;
	double	rap,v_poz,v_neg,t_rap,label;

	switch (InitialWeight)
	{
		case INITIAL_WEIGHT_ZERO:
			memset(pv.Weight,0,sizeof(double)*pv.Count);
			(pv.Bias) = 0.0;
			break;
		case INITIAL_WEIGHT_RANDOM:
			for (tr=0;tr<pv.Count;tr++)
				pv.Weight[tr]=((double)((rand()%201)-100))/100.0;
			(pv.Bias) = ((double)((rand()%201)-100))/100.0;
			break;
		case INITIAL_WEIGHT_STATISTICS:
			memset(pv.Weight,0,sizeof(double)*pv.Count);
			(pv.Bias) = 0.0;
			notif->Info("[%s] Computing initial weights statisticaly ... ",ObjectName);
			recCount = con->GetRecordCount();
			for (tr=0;tr<recCount;tr++)
			{
				if (con->GetRecord(MainRecord,tr)==false)
				{
					notif->Error("[%s] Unable to read record #%d ",ObjectName,tr);
					return false;
				}
				for (gr=0;gr<MainRecord.FeatCount;gr++)
				{
					pv.Weight[gr]+=learningRate * MainRecord.Features[gr]*MainRecord.Label;
				}
				(pv.Bias) += learningRate * MainRecord.Label;
			}
			break;
		case INITIAL_WEIGHT_RELEVANT:
			memset(pv.Weight,0,sizeof(double)*pv.Count);
			(pv.Bias) = 0.0;
			notif->Info("[%s] Computing initial weights in a relevant way ... ",ObjectName);
			recCount = con->GetRecordCount();
			v_poz = v_neg = 0;
			for (tr=0;tr<recCount;tr++)
			{
				if (con->GetRecordLabel(label,tr)==false)
				{
					notif->Error("[%s] Unable to read record #%d ",ObjectName,tr);
					return false;
				}
				if (label==1)
					v_poz++;
				else
					v_neg++;
			}
			for (tr=0;tr<recCount;tr++)
			{
				if (con->GetRecord(MainRecord,tr)==false)
				{
					notif->Error("[%s] Unable to read record #%d ",ObjectName,tr);
					return false;
				}
				for (rap=1,gr=0;gr<MainRecord.FeatCount;gr++)
					rap+=MainRecord.Features[gr];
				if (MainRecord.Label==1)
					t_rap = rap*v_poz;
				else
					t_rap = rap*v_neg;
				for (gr=0;gr<MainRecord.FeatCount;gr++)
				{
					pv.Weight[gr]+=(learningRate * MainRecord.Features[gr]*MainRecord.Label)/t_rap;
				}
				(pv.Bias) += (learningRate * MainRecord.Label)/t_rap;
			}
			break;
		case INITIAL_WEIGHT_FROMFILE:
			if (Load(pv,WeightFileName.GetText())==false)
				return false;
			break;
		default:
			notif->Error("Unknwo initial seright method : %d ",InitialWeight);
			return false;
	}
	return true;
}
bool	GenericPerceptron::LoadFeatureWeightFile()
{
	GML::Utils::AttributeList	attrList;
	UInt32						tr;

	if ((featWeight = new double[con->GetFeatureCount()])==NULL)
	{
		notif->Error("[%s] -> Unable to alloc featWeight[%d]",ObjectName,con->GetFeatureCount());
		return false;
	}
	if (attrList.Load(FeaturesWeightFile.GetText())==false)
	{
		notif->Error("[%s] -> Unable to load FeaturesWeightFile : %s",ObjectName,FeaturesWeightFile.GetText());
		return false;
	}
	if (attrList.Update("Weight",featWeight,sizeof(double)*con->GetFeatureCount())==false)
	{
		notif->Error("[%s] -> Unable to update 'Weight' property from %s",ObjectName,FeaturesWeightFile.GetText());
		return false;
	}
	notif->Info("[%s] -> %s loaded ok ",ObjectName,FeaturesWeightFile.GetText());
	// facem si un mic test

	for (tr=0;tr<con->GetFeatureCount();tr++)
	{
		if (featWeight[tr]<=0.0)
		{
			notif->Error("[%s] -> Feature Weight %d is lower than 0.0 (%.4lf)",ObjectName,tr,featWeight[tr]);
		}	
	}
	return true;
}
bool	GenericPerceptron::Init()
{
	// creez conexiunea
	if (InitConnections()==false)
		return false;

	if (CreateIndexes()==false)
		return false;
	// creez vectorii pentr Main si Best
	if (pvMain.Create(con->GetFeatureCount())==false)
	{
		notif->Error("[%s] -> Unable to create Main Perceptron Vector",ObjectName);
		return false;
	}
	if (pvBest.Create(con->GetFeatureCount())==false)
	{
		notif->Error("[%s] -> Unable to create Best Perceptron Vector",ObjectName);
		return false;
	}
	if (con->CreateMlRecord(MainRecord)==false)
	{
		notif->Error("[%s] -> Unable to create MainRecord ",ObjectName);
		return false;
	}
	BestResult.Clear();
	memset(&BestResult,0,sizeof(BestResult)); // ca sa fiu sigur ca si acc , se si sp sunt pe 0
	if (InitWeight(pvMain)==false)
		return false;

	// creez firele daca e cazul
	if (HasProperty("ThreadsCount"))
	{
		if (InitThreads()==false)
			return false;
		if (SplitMLThreadDataRange(RecordIndexes.Len())==false)
			return false;
	}
	
	if (adjustWeightMode==ADJUST_WEIGHT_USE_FEAT_WEIGHT)
	{
		if (LoadFeatureWeightFile()==false)
			return false;
	}

	return OnInit();
}

bool	GenericPerceptron::Train(PerceptronVector &pvTrain,PerceptronVector &pvTest,GML::Utils::Interval &range,GML::ML::MLRecord &rec, GML::Utils::Indexes *indexes)
{
	UInt32	*ptrIndex;
	UInt32	currentIndex,count,act_featCount;
	UInt32	nrFeatures = con->GetFeatureCount();
	double	sum,error;

	count = range.Size();
	if (indexes==NULL)
	{
		currentIndex = range.Start;
	} else {
		if (range.End>indexes->Len())
		{
			notif->Error("[%s] -> Invalid Range (%d..%d) for %d records",ObjectName,range.Start,range.End,indexes->Len());
			return false;
		}
		ptrIndex = indexes->GetList();
		ptrIndex += range.Start;
	}
	// use or not Bias
	if (!useB)
	{
		(pvTrain.Bias)=0;
		(pvTest.Bias)=0;
	}
	// train efectiv
	while (count>0)
	{
		if (indexes!=NULL)
			currentIndex = (*ptrIndex);
		// citesc elementul
		if (con->GetRecord(rec,currentIndex)==false)
		{
			notif->Error("[%s] -> Error reading record #%d",ObjectName,currentIndex);
			return false;
		}
		// calculez suma
		sum = GML::ML::VectorOp::ComputeVectorsSum(rec.Features,pvTest.Weight,nrFeatures)+(pvTest.Bias);
		// daca nu e corect clasificat , ajustam
		if ((sum * rec.Label)<=0)
		{
			switch (adjustWeightMode)
			{
				case ADJUST_WEIGHT_LEARNING_RATE:
					error = rec.Label * learningRate;
					break;
				case ADJUST_WEIGHT_USE_WEIGHT:
					error = rec.Label * rec.Weight * learningRate;
					break;
				case ADJUST_WEIGHT_LEASTMEANSQUARE:					
					if (sum==0)
						error = rec.Label * learningRate;
					else
						error = learningRate * (-sum);
					break;
				case ADJUST_WEIGHT_SPLIT_LEARNING_RATE:
					act_featCount = CountActiveFeatures(rec.Features,nrFeatures)+1; // +1 pentru Bias si ca sa fiu sigur ca e mai mare ca 0
					error = (rec.Label * learningRate) / ((double)act_featCount);
					break;
				case ADJUST_WEIGHT_SPLIT_LEASTMEANSQUARE:
					if (sum==0)
						error = rec.Label * learningRate;
					else
						error = -learningRate * sum;
					act_featCount = CountActiveFeatures(rec.Features,nrFeatures)+1; // +1 pentru Bias si ca sa fiu sigur ca e mai mare ca 0
					error = error / ((double)act_featCount);
					break;
				case ADJUST_WEIGHT_USE_FEAT_WEIGHT:
					error = rec.Label * learningRate;
					break;
				case ADJUST_WEIGHT_USE_POZITIVE_NEGATIVE_LEARNING_RATE:
					if (rec.Label==1)
						error = rec.Label * pozitiveLearningRate;
					else
						error = rec.Label * negativeLearningRate;
					break;
				default:
					notif->Error("[%s] -> Unknown value for property 'AdjustWeightMode' (%d) ",ObjectName,adjustWeightMode);
					return false;
			};
			
			if (adjustWeightMode==ADJUST_WEIGHT_USE_FEAT_WEIGHT)
			{
				GML::ML::VectorOp::AdjustPerceptronWeights(rec.Features,pvTrain.Weight,nrFeatures,error,featWeight);
				if (useB)
					(pvTrain.Bias) += error;
			} else {
				GML::ML::VectorOp::AdjustPerceptronWeights(rec.Features,pvTrain.Weight,nrFeatures,error);
				if (useB)
					(pvTrain.Bias) += error;
			}
		}

		// next element
		if (indexes!=NULL)
			ptrIndex++;
		else
			currentIndex++;
		count--;
	}
	return true;
}
bool	GenericPerceptron::Test(PerceptronVector &pvTest,GML::Utils::Interval &range,GML::ML::MLRecord &rec,GML::Utils::AlgorithmResult &Result,GML::Utils::Indexes *indexes,bool resetResult,bool resetResultTime)
{
	UInt32	*ptrIndex;
	UInt32	currentIndex,count;
	UInt32	nrFeatures = con->GetFeatureCount();

	if (resetResult)
		Result.Clear();
	if (resetResultTime)
		Result.time.Start();
	count = range.Size();
	if (indexes==NULL)
	{
		currentIndex = range.Start;
	} else {
		if (range.End>indexes->Len())
		{
			notif->Error("[%s] -> Invalid Range (%d..%d) for %d records",ObjectName,range.Start,range.End,indexes->Len());
			return false;
		}
		ptrIndex = indexes->GetList();
		ptrIndex += range.Start;
	}
	// use or not Bias
	if (!useB)
		(pvTest.Bias)=0;
	// train efectiv
	while (count>0)
	{
		if (indexes!=NULL)
			currentIndex = (*ptrIndex);
		// citesc elementul
		if (con->GetRecord(rec,currentIndex)==false)
		{
			notif->Error("[%s] -> Error reading record #%d",ObjectName,currentIndex);
			return false;
		}
		Result.Update(rec.Label==1,GML::ML::VectorOp::IsPerceptronTrained(rec.Features,pvTest.Weight,nrFeatures,pvTest.Bias,rec.Label));
		// next element
		if (indexes!=NULL)
			ptrIndex++;
		else
			currentIndex++;
		count--;
	}
	if (resetResult)
		Result.Compute();
	if (resetResultTime)
		Result.time.Stop();

	return true;
}
bool	GenericPerceptron::PerformTrain()
{
	UInt32							it,loopIterations;
	GML::Utils::GString				saveNM;
	GML::Utils::AlgorithmResult		Result;
	bool							bestUpdated;
	
	loopIterations = testAfterIterations;

	for (it=0;(it<maxIterations) && (!StopAlgorithm);it++)
	{
		Result.time.Start();		
		if (PerformTrainIteration(it)==false)
		{
			notif->Error("[%s] -> Error on training iteration ...",ObjectName);
			return false;
		}
		loopIterations--;
		if (loopIterations==0)
		{
			loopIterations = testAfterIterations;
			Result.Iteration = it;
			Result.Clear();
			if (PerformTestIteration(Result)==false)
			{
				notif->Error("[%s] -> Error on test iteration ...",ObjectName);
				return false;
			}
			Result.Compute();
			Result.time.Stop();
			notif->Result(Result);

			OnTestTerminateCondition(Result);
			if (OnUpdateBest(pvMain,Result,bestUpdated)==false)
				return false;
			if (bestUpdated)
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
				OnSaveBest(saveNM.GetText(),&BestResult);
			}
			if (saveData==SAVE_DATA_AFTER_EACH_ITERATION)
			{
				saveNM.SetFormated("%s_it_%d.txt",Name.GetText(),it+1);
				OnSaveData(saveNM.GetText(),&Result);
			}			
		}
	}
	if (saveData==SAVE_DATA_AT_FINISH)
	{
		saveNM.SetFormated("%s_final.txt",Name.GetText());
		OnSaveData(saveNM.GetText(),&Result);
	}
	return true;
}
bool	GenericPerceptron::PerformTest()
{
	GML::Utils::AlgorithmResult		Result;

	Result.Clear();
	Result.time.Start();
	if (PerformTestIteration(Result)==false)
	{
		notif->Error("[%s] -> Error on test iteration ...",ObjectName);
		return false;
	}	
	Result.Compute();
	Result.time.Stop();
	notif->Result(Result);
	return true;
}
void	GenericPerceptron::OnExecute()
{
	StopAlgorithm = false;
	
	switch (Command)
	{
		case COMMAND_NONE:
			notif->Info("[%s] -> Nothing to do ... ",ObjectName);
			return;
		case COMMAND_TRAIN:
			PerformTrain();
			return;
		case COMMAND_TEST:
			PerformTest();
			return;
	};
	notif->Error("[%s] -> Unkwnown command ID : %d",ObjectName,Command);
}

bool	GenericPerceptron::OnInit()
{
	return true;
}

void	GenericPerceptron::OnTestTerminateCondition(GML::Utils::AlgorithmResult &Result)
{
	if (Result.acc>=minimAcc)
	{
		notif->Info("[%s] -> Accuracy (Acc) exceded %2.3lf (%2.3lf) -> Terminating Algorithm !",ObjectName,minimAcc,Result.acc);
		StopAlgorithm = true;
		return;
	}
	if (Result.se>=minimSe)
	{
		notif->Info("[%s] -> Sensitivity (Se) exceded %2.3lf (%2.3lf) -> Terminating Algorithm !",ObjectName,minimSe,Result.se);
		StopAlgorithm = true;
		return;
	}
	if (Result.sp>=minimSp)
	{
		notif->Info("[%s] -> Specificity (Sp) exceded %2.3lf (%2.3lf) -> Terminating Algorithm !",ObjectName,minimSp,Result.sp);
		StopAlgorithm = true;
		return;
	}
}
bool	GenericPerceptron::OnUpdateBest(PerceptronVector &pv,GML::Utils::AlgorithmResult &Result,bool &bestUpdated)
{
	bestUpdated = false;
	switch (saveBest)
	{
		case SAVE_BEST_NONE:
			return true;
		case SAVE_BEST_ACC:
			if (Result.acc<BestResult.acc)
				return true;
			break;
		case SAVE_BEST_SE:
			if (Result.se<BestResult.se)
				return true;
			break;
		case SAVE_BEST_SP:
			if (Result.sp<BestResult.sp)
				return true;
			break;
		default:
			notif->Error("[%s] -> UpdateBest => false (code = %d) ",ObjectName,saveBest);
			return false;
	}
	if (pvBest.Copy(pv)==false)
	{
		notif->Error("[%s] -> Unable to update Best Result",ObjectName);
		return false;
	}
	BestResult.Copy(&Result);
	bestUpdated = true;
	return true;
}
bool	GenericPerceptron::OnSaveData(char *fileName,GML::Utils::AlgorithmResult *Result)
{
	return Save(pvMain,fileName,Result);
}
bool	GenericPerceptron::OnSaveBest(char *fileName,GML::Utils::AlgorithmResult *Result)
{
	return Save(pvBest,fileName,Result);
}