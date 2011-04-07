#include "BayesNaiv.h"

BayesNaiv::BayesNaiv()
{
	ObjectName = "BayesNaiv";

	LinkPropertyToString("Connector",strConector,"","Conectorul la care sa se conecteze");
	LinkPropertyToString("Notifier",strNotificator,"","Notificatorul la care sa se conecteze");
	LinkPropertyToString("DataBase",strDB,"","Baza de date la care sa se conecteze");	
	
	SetPropertyMetaData("Command","!!LIST:None=0,Train,Test!!");
}

bool BayesNaiv::Init()
{
	if ((notif = GML::Builder::CreateNotifier(strNotificator.GetText()))==NULL)
		return false;
	if (strDB.Len()==0)
	{
		if ((con = GML::Builder::CreateConnectors(strConector.GetText(),*notif))==NULL)
		{
			notif->Error("[%s] -> Unable to create Conector (%s)",ObjectName,strConector.GetText());
			return false;
		}
	} else {
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
	}
	return true;
}

unsigned int first4Decimals(double value)
{
	if((unsigned int)value == 1)
		return 0;

	if(value >= 0.0001)
		return (unsigned int)((value*1000000)/100);
	else return 1;
}

unsigned int mulPlusTrunc(unsigned int a, unsigned int b, unsigned int &exponent)
{
	unsigned int	prod;
	unsigned int	cat;

	prod = a*b;
	cat = prod/1000000;
	
	if(cat>9999)
	{
		exponent+=5;
		return prod/100000;
	}else if(cat > 999)
	{
		exponent+=4;
		return prod/10000;
	}else if (cat > 99)
	{
		exponent+=3;
		return prod/1000;
	}else if (cat > 9)
	{
		exponent+=2;
		return prod/100;
	} else if (cat >0){		
		exponent++;		
		return prod/10;	
	} else return prod;

}

bool BayesNaiv::PerformTest()
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

	pFeatCondInf = new double[con->GetFeatureCount()];
	pFeatCondClean = new double[con->GetFeatureCount()];

	a.Load("F:\\GMLib\\BayesNaivDB\\prob.txt");

	a.UpdateDouble("Pinf", pFileInf,true, 0.5);
	a.UpdateDouble("Pclean", pFileClean,true, 0.5);
	a.Update("pFeatCondClean", &pFeatCondClean[0], con->GetFeatureCount()*sizeof(double));
	a.Update("pFeatCondInf", &pFeatCondInf[0], con->GetFeatureCount()*sizeof(double));	

	result.Clear();
	a.Clear();

	for(i=0; i<con->GetRecordCount(); i++)
	{
		if(!(con->GetRecord(currentRecord, i)))
			continue;		
		//double pInf		= pFileInf*0.2;
		//double pInf		= pFileInf;			
		//double pClean	= pFileClean;		
		//double pInf		= 0.0;
		//double pClean	= 1.7;

		
		unsigned int pInf = first4Decimals(pFileInf*0.2);
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
				
				//pInf *= pFeatCondInf[j];
				//pClean *= pFeatCondClean[j];
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
			if((double)pInf/(pInf+pClean) > 0.999)
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
	unsigned int		nInf, nClean, nTotal;
	unsigned int		i,j;
	double				*pFeatCondInf;
	double				*pFeatCondClean;
	
	GML::ML::MLRecord currentRecord;
	vcInf = new unsigned int[con->GetFeatureCount()];
	vcClean = new unsigned int[con->GetFeatureCount()];
	pFeatCondInf = new double[con->GetFeatureCount()];
	pFeatCondClean = new double[con->GetFeatureCount()];

	if(!(con->CreateMlRecord(currentRecord)))
		return false;

	for(i=0; i<con->GetFeatureCount(); i++)
	{
		vcInf[i] = vcClean[i] = 0;
		pFeatCondInf[i] = pFeatCondClean[i] = 1.0;
	}
	totalInf = totalClean = 0;	
	nInf = nClean = nTotal = 0;

	notif->Info("[BTM] Start Working...");

	for(i=0; i<con->GetRecordCount(); i++)
	{
		if(!(con->GetRecord(currentRecord, i)))
			notif->Info("[BTM] error getting record");
		
		if(currentRecord.Label == -1)	//clean
		{
			totalClean++;			

			for(j=0; j<currentRecord.FeatCount; j++)			
				if((unsigned int)(currentRecord.Features[j]))	vcClean[j]++;							
		}
		else if(currentRecord.Label == 1){		//infected
			totalInf++;
			
			for(j=0; j<currentRecord.FeatCount; j++)
				if((unsigned int)(currentRecord.Features[j]))	vcInf[j]++;							
		}
	}

	notif->Info("[BTM] VC computed...");

	for(i=0; i<con->GetFeatureCount(); i++)
	{
		//if(vcClean[i]>3)				nClean++; 		
		//if(vcInf[i]>3)					nInf++;		
		//if(vcClean[i]>3 || vcInf[i]>3)	nTotal++;

		if(vcClean[i])				nClean++; 		
		if(vcInf[i])				nInf++;		
		if(vcClean[i] || vcInf[i])	nTotal++;		
	}



	notif->Info("[BTM] nTotal:%d  # nClean:%d # nInf: %d # totalClean:%d # totalInf:%d", nTotal, nClean, nInf, totalClean, totalInf);

	for(i=0; i<con->GetFeatureCount(); i++)
	{
		if(vcClean[i] || vcInf[i])		
		{			
			if(vcClean[i] + vcInf[i] < con->GetRecordCount()/4 )
			{
				pFeatCondClean[i] = (vcClean[i] + 1.0)/(totalClean + nTotal);
				pFeatCondInf[i] = (vcInf[i] + 1.0)/(totalInf + nTotal);				
			}
		}
	}

	GML::Utils::AttributeList	a;
	a.AddDouble("Pinf", (double)totalInf/con->GetRecordCount(), "asdfsaa");
	a.AddDouble("Pclean", (double)totalClean/con->GetRecordCount(), "asdfsaa");
	a.AddAttribute("pFeatCondClean",&pFeatCondClean[0],GML::Utils::AttributeList::DOUBLE,con->GetFeatureCount(),"kjf");
	a.AddAttribute("pFeatCondInf",&pFeatCondInf[0],GML::Utils::AttributeList::DOUBLE,con->GetFeatureCount(),"kjfddd");
	a.Save("F:\\GMLib\\BayesNaivDB\\prob.txt");	

	return true;
}

void BayesNaiv::OnExecute()
{
	switch (Command)
	{
		case COMMAND_NONE:
			notif->Info("[BTM] [%s] -> Nothing to do ... ",ObjectName);
			return;
		case COMMAND_TRAIN:
			notif->Info("[BTM] Command = Train");
			PerformTrain();
			return;
		case COMMAND_TEST:
			notif->Info("[BTM] Command = Test");
			PerformTest();
			return;
	};
	notif->Error("[BTM] [%s] -> Unkwnown command ID : %d",ObjectName,Command);
	
}
