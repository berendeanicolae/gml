#include "Projectron.h"


UInt32 Projectron::CountActiveFeatures(double *feat,UInt32 nrElements)
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

Projectron::Projectron()
{
	ObjectName = "Projectron";
	LinkPropertyToUInt32("KernelFunction",kernelFunction,POLYNOMIAL_KER,"!!LIST:Polinomiala=0,Sigmoida!!\nMetrica folosita");
}

bool Projectron::OnInit(){
	Space.Create(con->GetRecordCount());
	K.Create(0,0);
	return true;
}

bool Projectron::Train(PerceptronThreadData *ptd,GML::Utils::Indexes *indexes,bool clearDelta,bool addDeltaToPrimary){
	//to be implemented!
	
	UInt32	*ptrIndex = indexes->GetList();
	UInt32	count,act_featCount;
	UInt32	nrFeatures = con->GetFeatureCount();
	double	*w = ptd->Primary.Weight;
	double	*b = ptd->Primary.Bias;
	double	error;
	double	sum;

	ptd->corectelyClasify = 0;
	if (ptd->Range.End>indexes->Len())
	{
		notif->Error("[%s] -> (TRAIN)::Invalid Range (%d..%d) for thread %d with %d records",ObjectName,ptd->Range.Start,ptd->Range.End,ptd->ID,indexes->Len());
		return false;
	}
	ptrIndex += ptd->Range.Start;
	count=ptd->Range.Size();

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
		(*ptd->Primary.Bias)=0;

	while (count>0)
	{		
		if (con->GetRecord(ptd->Record,*ptrIndex)==false)
		{
			notif->Error("(TRAIN)::Error reading record #%d from thread #%d",(*ptrIndex),ptd->ID);
			return false;
		}
		sum = GML::ML::VectorOp::ComputeVectorsSum(ptd->Record.Features,ptd->Primary.Weight,nrFeatures)+(*ptd->Primary.Bias);

		if ((sum * ptd->Record.Label)<=0)
		{
			switch (adjustWeightMode)
			{
				case ADJUST_WEIGHT_LEARNING_RATE:
					error = ptd->Record.Label * learningRate;
					break;
				case ADJUST_WEIGHT_USE_WEIGHT:
					error = ptd->Record.Label * ptd->Record.Weight * learningRate;
					break;
				case ADJUST_WEIGHT_LEASTMEANSQUARE:					
					if (sum==0)
						error = ptd->Record.Label * learningRate;
					else
						error = -learningRate * sum;
					break;
				case ADJUST_WEIGHT_SPLIT_LEARNING_RATE:
					act_featCount = CountActiveFeatures(ptd->Record.Features,nrFeatures)+1; // +1 pentru Bias si ca sa fiu sigur ca e mai mare ca 0
					error = (ptd->Record.Label * learningRate) / ((double)act_featCount);
					break;
				case ADJUST_WEIGHT_SPLIT_LEASTMEANSQUARE:
					if (sum==0)
						error = ptd->Record.Label * learningRate;
					else
						error = -learningRate * sum;
					act_featCount = CountActiveFeatures(ptd->Record.Features,nrFeatures)+1; // +1 pentru Bias si ca sa fiu sigur ca e mai mare ca 0
					error = error / ((double)act_featCount);
					break;
				case ADJUST_WEIGHT_USE_FEAT_WEIGHT:
					error = ptd->Record.Label * learningRate;
					break;
				default:
					error = 0;
					break;
			};
			
			if (adjustWeightMode==ADJUST_WEIGHT_USE_FEAT_WEIGHT)
			{
				GML::ML::VectorOp::AdjustPerceptronWeights(ptd->Record.Features,w,nrFeatures,error,featWeight);
				if (useB)
					(*b) += error;
			} else {
				GML::ML::VectorOp::AdjustPerceptronWeights(ptd->Record.Features,w,nrFeatures,error);
				if (useB)
					(*b) += error;
			}
		} else {
			ptd->corectelyClasify ++;
		}
		count--;
		ptrIndex++;
	}
	if ((ptd->Delta.Weight!=NULL) && (addDeltaToPrimary))
		ptd->Primary.Add(ptd->Delta);
	return true;
}



bool Projectron::PerformTrainIteration()
{
	return Train(&FullData,&RecordIndexes,false,false);
}

bool Projectron::PerformTestIteration()
{
	return Test(&FullData);
}