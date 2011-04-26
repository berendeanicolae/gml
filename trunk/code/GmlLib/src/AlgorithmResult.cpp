#include "AlgorithmResult.h"

GML::Utils::AlgorithmResult::AlgorithmResult(void)
{
	Clear();
	Iteration = 0;
}
void GML::Utils::AlgorithmResult::Update(bool classType,bool corectellyClasified,double updateValue)
{
	if (corectellyClasified)
	{
		if (classType)
			tp+=updateValue;
		else
			tn+=updateValue;
	} else {
		if (classType)
			fn+=updateValue;
		else
			fp+=updateValue;
	}
}
void GML::Utils::AlgorithmResult::Clear()
{
    tp = tn = fp = fn = 0;
    sp = se = acc = med = 0.0;
	precision = fallout = FMeasure = 0.0;
}
void GML::Utils::AlgorithmResult::Compute()
{
	se = (tp * 100.0) / (double)(tp + fn);
	sp = (tn * 100.0) / (double)(tn + fp);
	acc = ((tp + tn) * 100.0) / (double)(tp + fp + tn + fn);
	med = (se+sp)/2;
	precision = (tp * 100.0)/(tp+fp);
	fallout = (fp * 100.0)/(tn+fp);
	FMeasure = (2 * precision * se)/(precision + se);
}
void GML::Utils::AlgorithmResult::Add(AlgorithmResult *res)
{
	if (res==NULL)
		return;
	tp += res->tp;
	tn += res->tn;
	fp += res->fp;
	fn += res->fn;
}
void GML::Utils::AlgorithmResult::Copy(AlgorithmResult *res)
{
	if (res==NULL)
		return;
    tp = res->tp;
    tn = res->tn;
    fn = res->fn;
    fp = res->fp;
    sp = res->sp;
    se = res->se;
    acc = res->acc;
	med = res->med;
	precision = res->precision;
	fallout = res->fallout;
	FMeasure = res->FMeasure;
}
