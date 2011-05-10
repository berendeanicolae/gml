	#include "BayesNaiv.h"

BayesNaiv::BayesNaiv()
{
	ObjectName = "BayesNaiv";

	LinkPropertyToUInt32("ProcIgnoreFeature",procIgnoreFeature,25,"A procent from where a flag is ignored");
	LinkPropertyToUInt32("PenaltyForInfFiles",penaltyForInfFile,100,"A procent to penalize the probability for infected files");
	LinkPropertyToUInt32("NrOfIterations", nrOfIterations, 1, "Number of iterations");
	LinkPropertyToDouble("ProcToSetVerdictInfected",procToSetInfected,0.999,"MinProbability to set the verdict=infected");
	LinkPropertyToBool("ClassicMul", classicMul, true, "Use normal multiplication method for probabilities");	
	LinkPropertyToString("ProbsFilePath", pathToProbsFile, "", "Path to the file to save the probabilities in");	
	LinkPropertyToString("PathBestProbsFile", pathBestProbsFile, "", "Path to the file to save the best probabilities in");
	LinkPropertyToBool("SaveBest", saveBest, false, "Save the trained probabilities");
	LinkPropertyToUInt32("BestClassifMethod", bestClassifMethod, BEST_NONE, "!!LIST:None=0,BEST_SE,BEST_SP,BEST_ACC!!");
	
	SetPropertyMetaData("Command","!!LIST:None=0,Train,Test!!");
}

bool BayesNaiv::Init()
{
	if (InitConnections()==false)
		return false;	

	pFeatCondInf = new double[con->GetFeatureCount()];
	pFeatCondClean = new double[con->GetFeatureCount()];
	for(int i=0; i<con->GetFeatureCount(); i++)
		pFeatCondInf[i] = pFeatCondClean[i] = 1.0;

	return InitThreads();
}

unsigned int first4Decimals(double value)
{
	if((unsigned int)value == 1)
		return 0;

	if(value >= 0.0001)
		return (unsigned int)((value*1000000)/100);
	else return 1;
}

unsigned int getNrOfDigits(unsigned int value)
{
	unsigned int nrC = 0;

	while(value)
	{
		value = value/10;
		nrC++;
	}

	return nrC;
}

unsigned int mulPlusTrunc(unsigned int a, unsigned int b, unsigned int &exponent)
{
	unsigned int	prod, cat, nrC;	

	prod = a*b;
	cat = prod/1000000;
	nrC = 0;
	
	
	if(cat)
		nrC = getNrOfDigits(cat);
	exponent += nrC;

	while(nrC--)
		prod = prod/10;

	return prod;	
}

bool BayesNaiv::BuildRecordsClassif(GML::Algorithm::MLThreadData &thData)
{
	GML::ML::MLRecord			currentRecord;
	int							i,j;
	int							clasif;

	if(!(con->CreateMlRecord(currentRecord)))
		return false;	

	for(i=thData.ThreadID; i<con->GetRecordCount(); i+=threadsCount)
	{
		if(!(con->GetRecord(currentRecord, i)))
		{
			notif->Info("[%s] -> error getting record #%d",ObjectName,i);
			return false;
		}
		
		double pInf		= pFileInf*penaltyForInfFile;
		double pClean	= pFileClean;							

		for(j=0; j<currentRecord.FeatCount; j++)
			if((unsigned int)currentRecord.Features[j])
			{								
				pInf *= pFeatCondInf[j];
				pClean *= pFeatCondClean[j];
			}

		clasif = -1;		
		if(pInf>pClean)
			if((double)pInf/(pInf+pClean) > procToSetInfected)
				clasif = 1;									
		
		result.Update(currentRecord.Label==1,clasif==currentRecord.Label);

		if(currentRecord.Label == 1)
			if(clasif == 1)
				recordsClassif[i] = TRUE_POSITIVE;
			else
				recordsClassif[i] = FALSE_NEGATIVE;
		else
			if(clasif == 1)
				recordsClassif[i] = FALSE_POSITIVE;
			else
				recordsClassif[i] = TRUE_NEGATIVE;
	}

	return true;
}

bool BayesNaiv::PerformTestClassicMul(bool toLoad, bool alloc, int iterNumber)
{
	result.Clear();
	result.time.Start();

	if(toLoad)
		LoadProbsFromFile();

	if(alloc)
		recordsClassif = new UInt8[con->GetRecordCount()];

	memset(recordsClassif, 0, sizeof(UInt8)*(con->GetRecordCount()));	
	ExecuteParalelCommand(Command);

	result.Compute();	
	result.time.Stop();
	result.Iteration = iterNumber;
	notif->Result(result);

	return true;
}

bool BayesNaiv::PerformTestNewMul()
{
	GML::Utils::AttributeList	a;

	double						*pFeatCondInf;
	double						*pFeatCondClean;
	double						pFileInf, pFileClean;
	unsigned int				i, j;
	int							clasif;
	GML::Utils::AlgorithmResult	result;
	GML::ML::MLRecord			currentRecord;

	if(!(con->CreateMlRecord(currentRecord)))
		return false;	

	if(pathToProbsFile.Equals(""))
	{
		notif->Error("[%s] Invalid path for file to load probabilities from", ObjectName);
		return false;
	}

	pFeatCondInf = new double[con->GetFeatureCount()];
	pFeatCondClean = new double[con->GetFeatureCount()];

	a.Load(pathToProbsFile);
	a.UpdateDouble("Pinf", pFileInf,true, 0.5);
	a.UpdateDouble("Pclean", pFileClean,true, 0.5);
	a.Update("pFeatCondClean", &pFeatCondClean[0], con->GetFeatureCount()*sizeof(double));
	a.Update("pFeatCondInf", &pFeatCondInf[0], con->GetFeatureCount()*sizeof(double));	

	result.Clear();	

	for(i=0; i<con->GetRecordCount(); i++)
	{
		if(!(con->GetRecord(currentRecord, i)))
		{
			notif->Info("[%s] -> error getting record #%d",ObjectName,i);
			return false;
		}				
		
		unsigned int pInf = first4Decimals((pFileInf*penaltyForInfFile)/100);
		unsigned int expPInf = 0;
		unsigned int pClean = first4Decimals(pFileClean);
		unsigned int expPClean = 0;
		
		clasif = -1;

		for(j=0; j<currentRecord.FeatCount; j++)
			if((unsigned int)currentRecord.Features[j])
			{	
				if(first4Decimals(pFeatCondInf[j]) == 0 && first4Decimals(pFeatCondClean[j]) == 0)
					continue;

				pInf = mulPlusTrunc(pInf, first4Decimals(pFeatCondInf[j]), expPInf);
				pClean = mulPlusTrunc(pClean, first4Decimals(pFeatCondClean[j]), expPClean);						
			}

		if(expPClean>expPInf)
		{
			pClean = pInf + 1;
		}else {
			if (expPInf - expPClean > 3)
				pClean = 0;
			else 
				pInf = pInf*pow(10.0, (int)(expPInf - expPClean));			
		}		
		
		if(pInf>pClean)
			if((double)pInf/(pInf+pClean) > procToSetInfected)
				clasif = 1;		
		
		result.Update(currentRecord.Label==1,clasif==currentRecord.Label);	
	}

	result.Compute();
	notif->Notify(100,&result,sizeof(result));	

	return true;
}

bool BayesNaiv::InitTrain()
{
	if (procIgnoreFeature>100)
	{
		notif->Error("[%s] Inavlid procent value (%d) ",ObjectName,procIgnoreFeature);
		return false;
	}

	if(pathToProbsFile.Equals(""))
	{
		notif->Error("[%s] Invalid path for file to save probabilities in", ObjectName);
		return false;
	}

	vcInf = new unsigned int[con->GetFeatureCount()];
	vcClean = new unsigned int[con->GetFeatureCount()];
	
	memset(vcInf, 0, sizeof(unsigned int)*con->GetFeatureCount());
	memset(vcClean, 0, sizeof(unsigned int)*con->GetFeatureCount());

	return true;
}

bool BayesNaiv::PerformTrain()
{
	unsigned int totalInf,totalClean;
	InitTrain();
	ExecuteParalelCommand(Command);
	//GDT: stiu ca am calculele pt. totalInf si totalClean facute pt. fiecare fir in parte
	//GDT: le adun

	totalInf = totalClean = 0;
	for (int tr=0;tr<threadsCount;tr++)
	{
		totalInf+=((BayesNaivThreadData *)ThData[tr].Context)->totalInf;
		totalClean+=((BayesNaivThreadData *)ThData[tr].Context)->totalClean;
	}

	//notif->Info("[%s] -> totalClean=%d totalInf=%d totalRecords=%d", ObjectName, totalClean, totalInf, con->GetRecordCount());
	BuildInitialProbabilities();	

	pFileInf = (double)totalInf/(totalInf + totalClean);
	pFileClean = (double)totalClean/(totalInf + totalClean);
	
	Command = COMMAND_TEST;
	PerformTestClassicMul(false, true);
	SaveProbsToFile(pathToProbsFile);

	unsigned int		*vcFNs;
	unsigned int		*vcFPs;
	int					copyOfNrIter = nrOfIterations;
	int					i;

	if(nrOfIterations>1)
	{
		vcFNs = vcInf;
		vcFPs = vcClean;
		vcFNsC = new UINT8[con->GetFeatureCount()];
		vcFPsC = new UINT8[con->GetFeatureCount()];
		memset(vcFNsC, 0, sizeof(UINT8)*con->GetFeatureCount());
		memset(vcFPsC, 0, sizeof(UINT8)*con->GetFeatureCount());

		bestIteration = 0.0;
	}

	while(--copyOfNrIter)
	{
		memset(vcFNs, 0, sizeof(unsigned int)*con->GetFeatureCount());
		memset(vcFPs, 0, sizeof(unsigned int)*con->GetFeatureCount());

		Command = COMMAND_COMPUTE_NEW_PROBS;
		ExecuteParalelCommand(Command);

		int maxApFPs		= 0;		
		int maxApFPsIndex	= 0;		
		int maxApFNs		= 0;		
		int maxApFNsIndex	= 0;		
		for(i=0; i<con->GetFeatureCount(); i++)
		{
			if((unsigned int)pFeatCondInf[i] == 1 || (unsigned int)pFeatCondClean[i] == 1)
				continue;

			if(maxApFPs < vcFPs[i])
			{
				maxApFPs = vcFPs[i];
				maxApFPsIndex = i;
			}
			if(maxApFNs < vcFNs[i])
			{
				maxApFNs = vcFNs[i];
				maxApFNsIndex = i;
			}			
		}

		//pFeatCondInf[maxApFPsIndex] *= 0.8;
		//pFeatCondClean[maxApFNsIndex] *= 0.9;
		vcFPsC[maxApFPsIndex]++;
		vcFNsC[maxApFNsIndex]++;
		if(vcFPsC[maxApFPsIndex]>5)
		{
			pFeatCondInf[maxApFPsIndex] = 1.0;
			pFeatCondClean[maxApFPsIndex] = 1.0;
		}
		else if(vcFNsC[maxApFNsIndex]>5)
		{
			pFeatCondInf[maxApFNsIndex] = 1.0;
			pFeatCondClean[maxApFNsIndex] = 1.0;
		}
		else
		{
			//pFeatCondInf[maxApFPsIndex] -= (double)vcFPs[maxApFPsIndex]/(result.tp+result.fp);
			//pFeatCondClean[maxApFNsIndex] -= (double)vcFNs[maxApFNsIndex]/(result.tn+result.fn);
			//pFeatCondClean[maxApFPsIndex] += (double)vcFPs[maxApFPsIndex]/(result.tp+result.fp);
			//pFeatCondInf[maxApFNsIndex] += (double)vcFNs[maxApFNsIndex]/(result.tn+result.fn);
			if(vcFPs[maxApFPsIndex]/(result.fp) > vcFNs[maxApFNsIndex]/(result.fn))
				//pFeatCondClean[maxApFPsIndex] += (double)vcFPs[maxApFPsIndex]/(result.fp);
				pFeatCondInf[maxApFPsIndex] *= 0.7;
			else
				//pFeatCondInf[maxApFNsIndex] += (double)vcFNs[maxApFNsIndex]/(result.fn);
				pFeatCondClean[maxApFNsIndex] *= 0.7;
		}

		Command = COMMAND_TEST;
		PerformTestClassicMul(false, false, nrOfIterations - copyOfNrIter);
		if(saveBest)
			SaveProbsIfBetter(&bestIteration);		
	}
	
	return true;
}

void BayesNaiv::SaveProbsIfBetter(double *best)
{
	double currentValue = 0.0;

	switch(bestClassifMethod)
	{
	case BEST_SE:
		currentValue = result.se;
		break;
	case BEST_SP:	
		currentValue = result.sp;
		break;
	case BEST_ACC:
		currentValue = result.acc;
		break;
	}

	if(*best < currentValue)
	{
		*best = currentValue; 
		GML::Utils::GString temp = pathBestProbsFile;
		notif->Info("----> %s", temp);
		SaveProbsToFile(temp);
		temp.Set("");
	}
}

bool BayesNaiv::ComputeWrongFeaturesFreq(GML::Algorithm::MLThreadData &thData)
{
	GML::ML::MLRecord			currentRecord;
	int							i,j;

	if(!(con->CreateMlRecord(currentRecord)))
		return false;	

	for(i=thData.ThreadID; i<con->GetRecordCount(); i+=threadsCount)
	{
		if(recordsClassif[i] == FALSE_POSITIVE)
		{
			if(!(con->GetRecord(currentRecord, i)))
			{
				notif->Info("[%s] -> error getting record #%d",ObjectName,i);
				return false;
			}

			for(j=0; j<currentRecord.FeatCount; j++)					
				if((unsigned int)currentRecord.Features[j])
					vcClean[j]++;
		} else if(recordsClassif[i] == FALSE_NEGATIVE)
		{
			if(!(con->GetRecord(currentRecord, i)))
			{
				notif->Info("[%s] -> error getting record #%d",ObjectName,i);
				return false;
			}

			for(j=0; j<currentRecord.FeatCount; j++)					
				if((unsigned int)currentRecord.Features[j])
					vcInf[j]++;
		}
	}

	return true;
}

bool BayesNaiv::ComputeFeaturesFreq(GML::Algorithm::MLThreadData &thData)
{		
	unsigned int		i,j;
	BayesNaivThreadData	*bntd = (BayesNaivThreadData*)thData.Context;
		
	GML::ML::MLRecord currentRecord;
	
	if(!(con->CreateMlRecord(currentRecord)))
		return false;

	if (thData.ThreadID==0)
		notif->StartProcent("[%s] -> Computing ... ",ObjectName);

	for(i=thData.ThreadID; i<con->GetRecordCount(); i+=threadsCount)
	{
		if(!(con->GetRecord(currentRecord, i)))
		{
			notif->Info("[%s] -> error getting record #%d",ObjectName,i);
			return false;
		}
		
		if(currentRecord.Label == -1)	//clean
		{
			bntd->totalClean++;

			for(j=0; j<currentRecord.FeatCount; j++)			
				if((unsigned int)(currentRecord.Features[j]))	
					vcClean[j]++;							
		}
		else if(currentRecord.Label == 1)	//infected
		{		
			bntd->totalInf++;

			for(j=0; j<currentRecord.FeatCount; j++)
				if((unsigned int)(currentRecord.Features[j]))	
					vcInf[j]++;							
		} 

		if (thData.ThreadID==0)
			notif->SetProcent(i,con->GetRecordCount());
	}

	if (thData.ThreadID==0)
		notif->EndProcent();

	return true;
}

void BayesNaiv::SaveProbsToFile(GML::Utils::GString filePath)
{
	GML::Utils::AttributeList	a;

	a.AddDouble("Pinf", pFileInf, "Prob for infected file");
	a.AddDouble("Pclean", pFileClean, "Prob for clean files");
	a.AddAttribute("pFeatCondClean",&pFeatCondClean[0],GML::Utils::AttributeList::DOUBLE,con->GetFeatureCount(),"Conditional probabilities for clean files");
	a.AddAttribute("pFeatCondInf",&pFeatCondInf[0],GML::Utils::AttributeList::DOUBLE,con->GetFeatureCount(),"Conditional probabilities for infected files");
	a.AddDouble("Se", result.se, "Se");
	a.AddDouble("Sp", result.sp, "Sp");
	a.AddDouble("Acc", result.acc, "Acc");

	a.Save(filePath);	
}

void BayesNaiv::LoadProbsFromFile()
{
	if(pathToProbsFile.Equals(""))
	{
		notif->Error("[%s] Invalid path for file to load probabilities from", ObjectName);		
	}

	GML::Utils::AttributeList	a;

	a.Load(pathToProbsFile);
	a.UpdateDouble("Pinf", pFileInf,true, 0.5);
	a.UpdateDouble("Pclean", pFileClean,true, 0.5);
	a.Update("pFeatCondClean", &pFeatCondClean[0], con->GetFeatureCount()*sizeof(double));
	a.Update("pFeatCondInf", &pFeatCondInf[0], con->GetFeatureCount()*sizeof(double));	
}

bool BayesNaiv::BuildInitialProbabilities()
{
	unsigned int		i;
	unsigned int		nTotal;
	
	nTotal = 0;

	for(i=0; i<con->GetFeatureCount(); i++)
	{		
		if(vcClean[i] || vcInf[i])	
			nTotal++;				
	}
	UInt32	flagValueToIgnore = (con->GetRecordCount()*procIgnoreFeature)/100;
	for(i=0; i<con->GetFeatureCount(); i++)
	{
		
		if(vcClean[i] || vcInf[i])		
		{			
			if(vcClean[i] + vcInf[i] <  flagValueToIgnore)
			{				
				pFeatCondClean[i] = (vcClean[i] + 1.0)/(con->GetFeatureCount() + nTotal);
				pFeatCondInf[i] = (vcInf[i] + 1.0)/(con->GetFeatureCount() + nTotal);		
			}
		}		
	}

	return true;
}

void BayesNaiv::OnExecute()
{
	switch (Command)
	{
		case COMMAND_NONE:
			notif->Info("[%s] -> Nothing to do ... ",ObjectName);
			return;
		case COMMAND_TRAIN:
			notif->Info("[%s] -> Training",ObjectName);		
			PerformTrain();
			return;
		case COMMAND_TEST:
			notif->Info("[%s] -> Testing",ObjectName);
			if(classicMul)
				PerformTestClassicMul(true, true);
			else
				PerformTestNewMul();
			return;
	};
	notif->Error("[%s] -> Unkwnown command ID : %d",ObjectName,Command);
	
}

void BayesNaiv::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand)
{
	switch(Command)
	{
		case COMMAND_TRAIN:
			ComputeFeaturesFreq(thData);
			break;
		case COMMAND_TEST:
			BuildRecordsClassif(thData);
			break;
		case COMMAND_COMPUTE_NEW_PROBS:
			ComputeWrongFeaturesFreq(thData);
			break;
	}
}

bool BayesNaiv::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	BayesNaivThreadData	*bntd = new BayesNaivThreadData();
	if (bntd==NULL)
		return false; // opritm astfel crearea de thread-uri

	thData.Context = bntd; // asociez obiectul nou creat threadului curent
	return true;
}