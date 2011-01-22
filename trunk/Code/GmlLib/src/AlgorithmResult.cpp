#include "StdAfx.h"
#include "AlgorithmResult.h"

GML::Utils::AlgorithmResult::AlgorithmResult(void)
{
	Clear();
}

void GML::Utils::AlgorithmResult::Clear()
{
    tp = tn = fp = fn = 0;
    sp = se = acc = 0.0;
	Iteration = 0;
}
void GML::Utils::AlgorithmResult::Compute()
{
	se = (tp * 100.0) / (double)(tp + fn);
	sp = (tn * 100.0) / (double)(tn + fp);
	acc = ((tp + tn) * 100.0) / (double)(tp + fp + tn + fn);
}
void GML::Utils::AlgorithmResult::Add(AlgorithmResult *res)
{
	if (res==NULL)
		return;
	tp += res->tp;
	tn += res->tn;
	fp += res->fn;
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
}
