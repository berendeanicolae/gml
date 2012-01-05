#include "FeatStats.h"

struct FunctionInfo
{
	char* 									Name;
	GML::ML::FeatStatComputeFunction		Addr;	
};

FunctionInfo fi[] = {
			{"TotalPoz", 		(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::Pozitive)},
			{"TotalNeg", 		(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::TotalPozitive)},
			{"PozCount", 		(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::ProcPozitive)},
			{"NegCount", 		(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::Negative)},
			{"PozProc", 		(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::TotalNegative)},
			{"NegProc", 		(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::ProcNegative)},
			{"ProcAverage", 	(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::RapPozNeg)},
			{"ProcTotal", 		(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::ProcDiff)},
			{"Poz/Neg", 		(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::ProcAverage)},
			{"ProcDiff", 		(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::ProcTotal)},
			{"AbsProcDiff", 	(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::AbsProcDiff)},
			{"Diff", 			(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::Diff)},
			{"AbsDiff", 		(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::AbsDiff)},
			{"F1", 				(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::F1)},
			{"F2", 				(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::F2)},
			{"InformationGain", (GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::ProcTo100)},
			{"ProcTo100", 		(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::AbsProcTo100)},
			{"AbsProcTo100", 	(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::InformationGain)},
			{"G1", 				(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::G1)},
			{"G2", 				(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::G2)},
			{"G3", 				(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::G3)},
			{"G4", 				(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::G4)},
			{"GProc", 			(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::GProc)},
			{"GProcTotal", 		(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::GProcTotal)},
			{"ProbPoz", 		(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::ProbPoz)},
			{"ProbNeg", 		(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::ProbNeg)},
			{"MaxProb", 		(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::MaxProb)},
			{"MedianClose", 	(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::MedianClosenest)},
			{"AsymUncertain", 	(GML::ML::FeatStatComputeFunction)&(GML::ML::FeatStatsFunctions::AsymetricUncertainty)}
		};

unsigned int GML::ML::FeatStatsFunctions::GetFunctionsCount()
{
	return sizeof(fi)/sizeof(FunctionInfo);
}

char* GML::ML::FeatStatsFunctions::GetFunctionName(unsigned int index)
{
	if (index >= (sizeof(fi)/sizeof(FunctionInfo)))
		return NULL;
	return fi[index].Name;	
}

GML::ML::FeatStatComputeFunction GML::ML::FeatStatsFunctions::GetFunctionPointer(unsigned int index)
{
	if (index >= (sizeof(fi)/sizeof(FunctionInfo)))
		return NULL;
	return fi[index].Addr;	
}

//====================================================================================================
double log2(double x)
{
	if (x==0) return 0;
	return log10(x) / (log10((double)2));
}
double entropy(double v_true,double v_false)
{
	if ((v_true+v_false)==0) return 0;
	double v1=((double)v_true)/((double)(v_true+v_false));
	double v2=((double)v_false)/((double)(v_true+v_false));
	return -(v1*log2(v1)) - (v2*log2(v2));
}
double GetBooleanEntropy(double pos_count, double neg_count)
{
    double pos_prob, neg_prob, ret_val = 0.0;
    double sum = pos_count + neg_count;

    if (sum < 1) sum = 1;

    pos_prob = (double)pos_count / (double)sum;
    neg_prob = (double)neg_count / (double)sum;

    if(neg_count > 0)
        ret_val -= neg_prob * log(neg_prob);

    if(pos_count > 0)
        ret_val -= pos_prob * log(pos_prob);

    return ret_val;
}

double GetJointBooleanEntropy(double lposfpos,double  lposfneg,double  lnegfpos,double  lnegfneg)
{
    double prob, ret_val = 0.0, sum = 0.0;

    sum += lposfpos;
    sum += lposfneg;
    sum += lnegfpos;
    sum += lnegfneg;

    if (sum < 1) sum = 1;
    
    prob = (double)lposfpos / (double)sum;
    if (prob) ret_val -= prob * log(prob);		

    prob = (double)lposfneg / (double)sum;
    if (prob) ret_val -= prob * log(prob);		

    prob = (double)lnegfpos / (double)sum;
    if (prob)ret_val -= prob * log(prob);		

    prob = (double)lnegfneg / (double)sum;
    if (prob)ret_val -= prob * log(prob);		

    return ret_val;
}
//====================================================================================================

double GML::ML::FeatStatsFunctions::Pozitive(FeatureInformation *f)
{
	return f->countPozitive;
}
double GML::ML::FeatStatsFunctions::TotalPozitive(FeatureInformation *f)
{
	return f->totalPozitive;
}
double GML::ML::FeatStatsFunctions::ProcPozitive(FeatureInformation *f)
{
	if (f->totalPozitive==0)
		return 0;
	return (f->countPozitive*100.0)/f->totalPozitive;
}
double GML::ML::FeatStatsFunctions::Negative(FeatureInformation *f)
{
	return f->countNegative;
}
double GML::ML::FeatStatsFunctions::TotalNegative(FeatureInformation *f)
{
	return f->totalNegative;
}
double GML::ML::FeatStatsFunctions::ProcNegative(FeatureInformation *f)
{
	if (f->totalNegative==0)
		return 0;
	return (f->countNegative*100.0)/f->totalNegative;
}
double GML::ML::FeatStatsFunctions::RapPozNeg(FeatureInformation *f)
{
	if (f->countPozitive==f->countNegative)
		return 0;
	if (f->countPozitive>f->countNegative)
		return f->countPozitive/(f->countNegative+1);
	else
		return -(f->countNegative/(f->countPozitive+1));
}
double GML::ML::FeatStatsFunctions::ProcDiff(FeatureInformation *f)
{
	return ((f->countPozitive*100)/f->totalPozitive)-((f->countNegative*100)/f->totalNegative);
}
double GML::ML::FeatStatsFunctions::ProcAverage(FeatureInformation *f)
{
	if ((f->totalNegative==0) || (f->totalPozitive==0))
		return 0;		
	return (((f->countPozitive*100)/f->totalPozitive)+((f->countNegative*100)/f->totalNegative))/2;
}
double GML::ML::FeatStatsFunctions::ProcTotal(FeatureInformation *f)
{
	if ((f->totalNegative+f->totalPozitive)==0)
		return 0;		
	return ((f->countPozitive+f->countNegative)*100)/(f->totalNegative+f->totalPozitive);	
}
double GML::ML::FeatStatsFunctions::AbsProcDiff(FeatureInformation *f)
{
	if ((f->countPozitive==0) && (f->countNegative==0))
		return 0;
	if ((f->totalNegative==0) || (f->totalPozitive==0))
		return 100;
	return abs(((f->countPozitive*100)/f->totalPozitive)-((f->countNegative*100)/f->totalNegative));
}
double GML::ML::FeatStatsFunctions::Diff(FeatureInformation *f)
{
	return f->countPozitive-f->countNegative;
}
double GML::ML::FeatStatsFunctions::AbsDiff(FeatureInformation *f)
{
	if (f->countPozitive>f->countNegative)
		return f->countPozitive-f->countNegative;
	else
		return f->countNegative-f->countPozitive;
}
double GML::ML::FeatStatsFunctions::F1(FeatureInformation *f)
{
	double t_mal = f->countPozitive;
	double f_mal = f->totalPozitive - t_mal;
	double t_clean = f->countNegative;
	double f_clean = f->totalNegative - t_clean;

	double all_mal = f->totalPozitive;
	double all_clean = f->totalNegative;
	double miu_pl = (double)t_mal / all_mal;
	double miu_min = (double)t_clean / all_clean;
	double sigma_pl = sqrt((double)(t_mal * (1 - miu_pl) * (1 - miu_pl) + f_mal * miu_pl * miu_pl));
	double sigma_min = sqrt((double)(t_clean * (1 - miu_min) * (1 - miu_min) + f_clean * miu_min * miu_min));
	double v1 = miu_pl - miu_min;
	double v2 = sigma_pl + sigma_min;
	//if (v1 < 0) v1 = -v1;
	//if (v2 < 0) v2 = -v2;
	if (t_mal + t_clean == 0) return 0;
	return (v1*1000000) / v2;
	//double precision = (double)(f.t_mal) / ((double)f.t_mal);
}
double GML::ML::FeatStatsFunctions::F2(FeatureInformation *f)
{
	double t_mal = f->countPozitive;
	double f_mal = f->totalPozitive - t_mal;
	double t_clean = f->countNegative;
	double f_clean = f->totalNegative - t_clean;

	if ((t_mal==0) && (t_clean == 0)) return 0;
	if ((f->totalPozitive==0) || (f->totalNegative==0)) return 0;

	double all_mal = f->totalPozitive;
	double all_clean = f->totalNegative;
    double miu_pl = (double)t_mal / all_mal;
    double miu_min = (double)t_clean / all_clean;
    double miu_total = (double)(t_mal + t_clean) / (all_mal + all_clean);
    double sigma_pl = sqrt((double)(t_mal * (1 - miu_pl) * (1 - miu_pl) + f_mal * miu_pl * miu_pl));
    double sigma_min = sqrt((double)(t_clean * (1 - miu_min) * (1 - miu_min) + f_clean * miu_min * miu_min));
    double v1 = (miu_pl - miu_total) * (miu_pl - miu_total) + (miu_min - miu_total) * (miu_min - miu_total);
    double v2 = sigma_pl*sigma_pl + sigma_min*sigma_min;
    
	if (v2==0) return 0;
    return (v1 *1000000)/ v2;
}
double GML::ML::FeatStatsFunctions::ProcTo100(FeatureInformation *f)
{
	if (f->countPozitive>f->countNegative)
		return 100-(f->countNegative/f->countPozitive)*100;
	if (f->countPozitive<f->countNegative)
		return -(100-(f->countPozitive/f->countNegative)*100);
	return 0;
}
double GML::ML::FeatStatsFunctions::AbsProcTo100(FeatureInformation *f)
{
	if (f->countPozitive>f->countNegative)
		return 100-(f->countNegative/f->countPozitive)*100;
	if (f->countPozitive<f->countNegative)
		return 100-(f->countPozitive/f->countNegative)*100;
	return 0;
}

double GML::ML::FeatStatsFunctions::InformationGain(FeatureInformation *f)
{
	double e, e1, e2;
	double total = f->totalPozitive+f->totalNegative;
	e = entropy(f->countPozitive + (f->totalNegative-f->countNegative), f->countNegative + (f->totalNegative-f->countNegative));
	e1 = entropy(f->countPozitive, f->countNegative);
	e2 = entropy(f->countPozitive, (f->totalNegative-f->countNegative));

	return e - ((double)(f->countPozitive + f->countNegative) / total) * e1 - ((double)((f->totalPozitive-f->countPozitive) + (f->totalNegative-f->countNegative)) / total) * e2;
}
double GML::ML::FeatStatsFunctions::G1(FeatureInformation *f)
{
	double total = f->countPozitive+f->countNegative;
	double dif = abs(f->countPozitive - f->countNegative);
	
	if (total==0)
		return 0;
	return (dif * 100)/total;
}
double GML::ML::FeatStatsFunctions::G2(FeatureInformation *f)
{
	double all = f->totalPozitive+f->totalNegative;
	double total = f->countPozitive+f->countNegative;
	double dif = abs(f->countPozitive - f->countNegative);
	
	if (total==0)
		return 0;
	return ((dif * 100)/total)*((dif*100)/all);
}
double GML::ML::FeatStatsFunctions::G3(FeatureInformation *f)
{
	double all = f->totalPozitive+f->totalNegative;
	double total = f->countPozitive+f->countNegative;
	double dif = abs(f->countPozitive - f->countNegative);
	
	if (total==0)
		return 0;
	return (((dif * 100)/total)+((dif*100)/all));
}
double GML::ML::FeatStatsFunctions::G4(FeatureInformation *f)
{
	double total = f->totalPozitive+f->totalNegative;
	double dif = abs(f->countPozitive - f->countNegative);
	
	if (total==0)
		return 0;
	return (dif * 100)/total;
}
double GML::ML::FeatStatsFunctions::GProc(FeatureInformation *f)
{
	double p_poz,p_neg,dif;

	if ((f->totalPozitive==0.0) || (f->totalNegative==0))
		return 0;	// functioneaza doar daca am de ambele feluri
	if ((f->countPozitive==0) && (f->countNegative==0))
		return 0;
	p_poz = (f->countPozitive*100.0)/f->totalPozitive;
	p_neg = (f->countNegative*100.0)/f->totalNegative;
	dif = abs(p_poz-p_neg);
	return (dif * 100.0)/(p_poz+p_neg);
}
double GML::ML::FeatStatsFunctions::GProcTotal(FeatureInformation *f)
{
	double p_poz,p_neg,dif,p_total;

	if ((f->totalPozitive==0.0) || (f->totalNegative==0))
		return 0;	// functioneaza doar daca am de ambele feluri
	if ((f->countPozitive==0) && (f->countNegative==0))
		return 0;
	p_poz = (f->countPozitive*100.0)/f->totalPozitive;
	p_neg = (f->countNegative*100.0)/f->totalNegative;
	p_total = ((f->countNegative+f->countPozitive)*100.0)/(f->totalNegative+f->totalPozitive);
	dif = abs(p_poz-p_neg);
	return dif*p_total;
}
double GML::ML::FeatStatsFunctions::ProbPoz(FeatureInformation *f)
{
	if ((f->totalPozitive==0) || (f->totalNegative==0))
		return 0;
	double prob_mal = f->countPozitive/f->totalPozitive;
	double prob_cln = f->countNegative/f->totalNegative;
	return prob_mal*(1-prob_cln);
}
double GML::ML::FeatStatsFunctions::ProbNeg(FeatureInformation *f)
{
	if ((f->totalPozitive==0) || (f->totalNegative==0))
		return 0;
	double prob_mal = f->countPozitive/f->totalPozitive;
	double prob_cln = f->countNegative/f->totalNegative;
	return prob_cln*(1-prob_mal);
}
double GML::ML::FeatStatsFunctions::MaxProb(FeatureInformation *f)
{
	return max(ProbPoz(f),ProbNeg(f));
}
double GML::ML::FeatStatsFunctions::MedianClosenest(FeatureInformation *f)
{
	if ((f->totalPozitive==0) || (f->totalNegative==0))
		return 0;
	double prob_mal = f->countPozitive/f->totalPozitive-0.5;
	double prob_cln = f->countNegative/f->totalNegative-0.5;
	if (prob_mal<0)
		prob_mal = -prob_mal;
	if (prob_cln<0)
		prob_cln = -prob_cln;
	return ((1-prob_mal)+(1-prob_cln))/2;
}

double GML::ML::FeatStatsFunctions::AsymetricUncertainty(FeatureInformation *f) 
{
    double feature_entropy, label_entropy, joint_entropy, ret_val;
    
    double lposfpos = f->countPozitive;
    double lposfneg = f->totalPozitive - f->countPozitive;
    double lnegfpos = f->countNegative;
    double lnegfneg = f->totalNegative - f->countNegative;

    feature_entropy = GetBooleanEntropy(f->countPozitive+f->countNegative, f->totalNegative+f->totalPozitive);
    label_entropy   = GetBooleanEntropy(f->totalPozitive, f->totalNegative);
    joint_entropy   = GetJointBooleanEntropy(lposfpos, lposfneg, lnegfpos, lnegfneg);

    ret_val = feature_entropy + label_entropy;
    if (ret_val < 1e-10) ret_val = 1e-10;
    ret_val = 2.0 * (ret_val - joint_entropy) / ret_val;
    return ret_val;
}
