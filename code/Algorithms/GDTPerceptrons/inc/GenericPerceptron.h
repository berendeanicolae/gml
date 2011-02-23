#ifndef __GENERIC_PERCEPTRON__
#define __GENERIC_PERCEPTRON__

#include "gmllib.h"

struct PerceptronThreadData
{
	GML::ML::MLRecord				Record;
	double							*Weight;
	double							*Delta;
	double							b_Weight;
	double							b_Delta;
	UInt32							ID;
	GML::Utils::AlgorithmResult		Res;
	UInt32							*RecordIndexes;
	UInt32							RecordIndexesCount;
};

class GenericPerceptron : public GML::Algorithm::IAlgorithm
{
	GML::DB::IDataBase				*db;
	GML::ML::IConector				*con;

	// proprietati
	GML::Utils::GString				Name;
	GML::Utils::GString				Conector;
	GML::Utils::GString				DataBase;
	GML::Utils::GString				Notifier;
	GML::Utils::GString				WeightFileName;
	GML::Utils::GString				InitialWeight;

	// proprietati de training
	double							learningRate;
	bool							useWeight;
	bool							useB;
	bool							batchPerceptron;
	UInt32							testAfterIterations;


	// proprietati de oprire
	double							minimAcc;
	double							minimSe;
	double							minimSp;
	UInt32							maxIterations;

	// fire
	UInt32							threadsCount;
	

	// Thread data
	PerceptronThreadData			*ptData;
	GML::Utils::ThreadParalelUnit	*tpu;

protected:
	bool					Train(PerceptronThreadData *ptd);
	bool					Test(PerceptronThreadData *ptd);
public:
	GenericPerceptron();

	bool					Init();
};


#endif
