#include "BayesNaiv.h"

BayesNaiv::BayesNaiv()
{
	ObjectName = "BayesNaiv";

	LinkPropertyToUInt32("ProcIgnoreFeature",procIgnoreFeature,25,"A procent from where a flag is ignored");
	LinkPropertyToUInt32("PenaltyForInfFiles",penaltyForInfFile,100,"A procent to penalize the probability for infected files");
	LinkPropertyToDouble("ProcToSetVerdictInfected",procToSetInfected,0.999,"MinProbability to set the verdict=infected");
	LinkPropertyToBool("ClassicMul", classicMul, true, "Use normal multiplication method for probabilities");	
	LinkPropertyToString("ProbsFilePath", pathToProbsFile, "", "Path to the file to save the probabilities in");
	
	SetPropertyMetaData("Command","!!LIST:None=0,Train,Test!!");
}

bool BayesNaiv::Init()
{
	return InitConnections();
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

bool BayesNaiv::PerformTestClassicMul()
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
	result.time.Start();
	a.Clear();

	for(i=0; i<con->GetRecordCount(); i++)
	{
		if(!(con->GetRecord(currentRecord, i)))
		{
			notif->Info("[%s] -> error getting record #%d",ObjectName,i);
			return false;
		}			
		double pInf		= pFileInf*penaltyForInfFile;
		double pClean	= pFileClean;							

		clasif = -1;

		for(j=0; j<currentRecord.FeatCount; j++)
			if((unsigned int)currentRecord.Features[j])
			{								
				pInf *= pFeatCondInf[j];
				pClean *= pFeatCondClean[j];
			}

		if(pInf>pClean)
			if((double)pInf/(pInf+pClean) > procToSetInfected)
				clasif = 1;		
		
		result.Update(currentRecord.Label==1,clasif==currentRecord.Label);	
	}

	result.Compute();
	result.time.Stop();
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

bool BayesNaiv::PerformTrain()
{
	unsigned int		*vcInf;
	unsigned int		*vcClean;
	unsigned int		totalInf, totalClean;
	unsigned int		nTotal;
	unsigned int		i,j;
	double				*pFeatCondInf;
	double				*pFeatCondClean;
	
	GML::ML::MLRecord currentRecord;
	vcInf = new unsigned int[con->GetFeatureCount()];
	vcClean = new unsigned int[con->GetFeatureCount()];
	pFeatCondInf = new double[con->GetFeatureCount()];
	pFeatCondClean = new double[con->GetFeatureCount()];

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

	if(!(con->CreateMlRecord(currentRecord)))
		return false;

	for(i=0; i<con->GetFeatureCount(); i++)
	{
		vcInf[i] = vcClean[i] = 0;
		pFeatCondInf[i] = pFeatCondClean[i] = 1.0;
	}
	totalInf = totalClean = 0;	
	nTotal = 0;

	for(i=0; i<con->GetRecordCount(); i++)
	{
		if(!(con->GetRecord(currentRecord, i)))
		{
			notif->Info("[%s] -> error getting record #%d",ObjectName,i);
			return false;
		}
		
		if(currentRecord.Label == -1)	//clean
		{
			totalClean++;			

			for(j=0; j<currentRecord.FeatCount; j++)			
				if((unsigned int)(currentRecord.Features[j]))	
					vcClean[j]++;							
		}
		else if(currentRecord.Label == 1){		//infected
			totalInf++;
			
			for(j=0; j<currentRecord.FeatCount; j++)
				if((unsigned int)(currentRecord.Features[j]))	
					vcInf[j]++;							
		}
	}	

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
				pFeatCondClean[i] = (vcClean[i] + 1.0)/(totalClean + nTotal);
				pFeatCondInf[i] = (vcInf[i] + 1.0)/(totalInf + nTotal);				
			}
		}
	}

	GML::Utils::AttributeList	a;

	a.AddDouble("Pinf", (double)totalInf/con->GetRecordCount(), "Prob for infected file");
	a.AddDouble("Pclean", (double)totalClean/con->GetRecordCount(), "Prob for clean files");
	a.AddAttribute("pFeatCondClean",&pFeatCondClean[0],GML::Utils::AttributeList::DOUBLE,con->GetFeatureCount(),"Conditional probabilities for clean files");
	a.AddAttribute("pFeatCondInf",&pFeatCondInf[0],GML::Utils::AttributeList::DOUBLE,con->GetFeatureCount(),"Conditional probabilities for infected files");

	a.Save(pathToProbsFile);	

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
				PerformTestClassicMul();
			else
				PerformTestNewMul();
			return;
	};
	notif->Error("[%s] -> Unkwnown command ID : %d",ObjectName,Command);
	
}
