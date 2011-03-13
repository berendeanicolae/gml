#ifndef __GENERIC_PERCEPTRON__
#define __GENERIC_PERCEPTRON__

#include "gmllib.h"

struct PerceptronVector
{
	double	*Weight;
	double	*Bias;
	UInt32	Count;
public:
	PerceptronVector();
	~PerceptronVector();

	bool	Create(UInt32 count);
	bool	Create(PerceptronVector &pv);
	void	Destroy();
	void	Add(PerceptronVector &pv);
};
struct PerceptronThreadData
{
	GML::ML::MLRecord				Record;
	PerceptronVector				Primary;
	PerceptronVector				Delta;
	UInt32							ID;
	GML::Utils::AlgorithmResult		Res;
	GML::Utils::Interval			Range;
	void							*ExtraData;
public:
	PerceptronThreadData();
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
	enum {
		ADJUST_WEIGHT_LEARNING_RATE = 0,
		ADJUST_WEIGHT_USE_WEIGHT,
		ADJUST_WEIGHT_LEASTMEANSQUARE,
		ADJUST_WEIGHT_SPLIT_LEARNING_RATE,
		ADJUST_WEIGHT_SPLIT_LEASTMEANSQUARE,
		ADJUST_WEIGHT_USE_FEAT_WEIGHT,
		
	};
protected:
	GML::DB::IDataBase				*db;
	GML::ML::IConnector				*con;

	// proprietati
	GML::Utils::GString				Name;
	GML::Utils::GString				Conector;
	GML::Utils::GString				DataBase;
	GML::Utils::GString				Notifier;
	GML::Utils::GString				WeightFileName;
	GML::Utils::GString				FeaturesWeightFile;
	UInt32							InitialWeight;

	// proprietati de training
	double							learningRate;
	bool							useB;
	bool							batchPerceptron;
	UInt32							saveData;
	UInt32							saveBest;
	UInt32							testAfterIterations;
	UInt32							adjustWeightMode;

	// proprietati de oprire
	double							minimAcc;
	double							minimSe;
	double							minimSp;
	UInt32							maxIterations;

	// fire
	UInt32							threadsCount;
	

	// Thread data	
	GML::Utils::Indexes				RecordIndexes;
	PerceptronThreadData			FullData,BestData;
	GML::Utils::ThreadParalelUnit	*tpu;
	double							*featWeight;
	

public:
	PerceptronThreadData			*ptData;
protected:
	bool					Train(PerceptronThreadData *ptd,bool clearDelta,bool addDeltaToPrimary);
	bool					Train(PerceptronThreadData *ptd,GML::Utils::Indexes *recordIndexes,bool clearDelta,bool addDeltaToPrimary);
	
	bool					Test(PerceptronThreadData *ptd);
	bool					Test(PerceptronThreadData *ptd,GML::Utils::Indexes *recordIndexes);
	
	bool					SplitInterval(PerceptronThreadData *ptd,UInt32 ptdElements,GML::Utils::Interval &interval);
	bool					Create(PerceptronThreadData &ptd,UInt32 id,PerceptronThreadData *original=NULL);
	bool					CreateIndexes();
	bool					UpdateBest(PerceptronThreadData &ptd);
	bool					Save(PerceptronThreadData &ptd,char *fileName);
	bool					Load(PerceptronThreadData &ptd,char *fileName);
	bool					LoadFeatureWeightFile();
	bool					InitWeight(PerceptronThreadData &ptd);
	bool					ExecuteParalelCommand(UInt32 command);
	void					CheckTerminateCondition(PerceptronThreadData &ptd);

	virtual void			OnTestTerminateCondition(PerceptronThreadData &ptd);
	virtual bool			OnInit();
	virtual bool			OnUpdateBestData();
	virtual bool			OnSaveData(char *fileName);		

	virtual bool			PerformTrainIteration()=0;
	virtual bool			PerformTestIteration()=0;

public:
	GenericPerceptron();

	virtual void			OnRunThreadCommand(PerceptronThreadData &ptd,UInt32 command) {};

	bool					Init();
	bool					PerformTrain();
	bool					PerformTest();
	void					OnExecute(char *command);
};


#endif
