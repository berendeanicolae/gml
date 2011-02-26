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
protected:
	enum {
		INITIAL_WEIGHT_ZERO  = 0,
		INITIAL_WEIGHT_RANDOM,
		INITIAL_WEIGHT_FROMFILE
	};
	enum {
		SAVE_BEST_NONE = 0,
		SAVE_BEST_ACC,
		SAVE_BEST_SE,
		SAVE_BEST_SP,
	};
	enum {
		SAVE_DATA_NONE = 0,
		SAVE_DATA_AFTER_EACH_ITERATION,
		SAVE_DATA_AT_FINISH,
	};
protected:
	GML::DB::IDataBase				*db;
	GML::ML::IConector				*con;

	// proprietati
	GML::Utils::GString				Name;
	GML::Utils::GString				Conector;
	GML::Utils::GString				DataBase;
	GML::Utils::GString				Notifier;
	GML::Utils::GString				WeightFileName;
	UInt32							InitialWeight;

	// proprietati de training
	double							learningRate;
	bool							useWeight;
	bool							useB;
	bool							batchPerceptron;
	UInt32							saveData;
	UInt32							saveBest;
	UInt32							testAfterIterations;


	// proprietati de oprire
	double							minimAcc;
	double							minimSe;
	double							minimSp;
	UInt32							maxIterations;

	// fire
	UInt32							threadsCount;
	

	// Thread data	
	PerceptronThreadData			FullData,BestData;
	GML::Utils::ThreadParalelUnit	*tpu;
public:
	PerceptronThreadData			*ptData;
protected:
	bool					Train(PerceptronThreadData *ptd);
	bool					Test(PerceptronThreadData *ptd);
	bool					SplitIndexes(PerceptronThreadData *ptd,UInt32 ptdElements,PerceptronThreadData *original);
	bool					Create(PerceptronThreadData &ptd,UInt32 id);
	bool					UpdateBest(PerceptronThreadData &ptd);
	bool					Save(PerceptronThreadData &ptd,char *fileName);
	bool					Load(PerceptronThreadData &ptd,char *fileName);
	bool					InitWeight(PerceptronThreadData &ptd);
	bool					ExecuteParalelCommand(UInt32 command);


	virtual bool			PerformTrainIteration()=0;
	virtual bool			PerformTestIteration()=0;
	virtual bool			OnUpdateBestData()=0;
	virtual bool			OnSaveData(char *fileName)=0;		

public:
	GenericPerceptron();

	virtual void			OnRunThreadCommand(PerceptronThreadData &ptd,UInt32 command) {};

	bool					Init();
	bool					PerformTrain();
	bool					PerformTest();
	void					OnExecute(char *command);
};


#endif
