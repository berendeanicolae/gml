#ifndef __GENERIC_PERCEPTRON__
#define __GENERIC_PERCEPTRON__

#include "gmllib.h"

struct PerceptronVector
{
	double	*Weight;
	double	Bias;
	UInt32	Count;
public:
	PerceptronVector();
	~PerceptronVector();

	bool	Create(UInt32 count);
	bool	Copy(PerceptronVector &pv);
	void	ResetValues();
	void	Destroy();
	void	Add(PerceptronVector &pv);
};

class GenericPerceptron : public GML::Algorithm::IMLAlgorithm
{
protected:
	enum {
		COMMAND_NONE = 0,
		COMMAND_TRAIN,
		COMMAND_TEST
	};
	enum {
		INITIAL_WEIGHT_ZERO  = 0,
		INITIAL_WEIGHT_RANDOM,
		INITIAL_WEIGHT_STATISTICS,
		INITIAL_WEIGHT_RELEVANT,
		INITIAL_WEIGHT_FROMFILE
	};
	enum {
		SAVE_BEST_NONE = 0,
		SAVE_BEST_ACC,
		SAVE_BEST_SE,
		SAVE_BEST_SP,
		SAVE_BEST_MED,
		SAVE_BEST_TP,
		SAVE_BEST_TN,
		SAVE_BEST_FP,
		SAVE_BEST_FN
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
		ADJUST_WEIGHT_USE_POZITIVE_NEGATIVE_LEARNING_RATE,		
	};
protected:
	// proprietati
	GML::Utils::GString				ResultsName;
	GML::Utils::GString				WeightFileName;
	GML::Utils::GString				FeaturesWeightFile;
	UInt32							InitialWeight;

	// proprietati de training
	double							learningRate;
	double							pozitiveLearningRate;
	double							negativeLearningRate;
	double							power;
	bool							useB;
	bool							usePolinomialFunction;
	UInt32							saveData;
	UInt32							saveBest;
	UInt32							testAfterIterations;
	UInt32							adjustWeightMode;

	// proprietati de oprire
	double							minimAcc;
	double							minimSe;
	double							minimSp;
	UInt32							maxIterations;

		
	GML::Utils::GTFVector<UInt32>	activeFeatCount;
	GML::Utils::Indexes				RecordIndexes;
	GML::Utils::AlgorithmResult		BestResult;
	GML::ML::MLRecord				MainRecord;
	PerceptronVector				pvBest,pvMain;

	UInt32							StartIteration;


	double							*featWeight;
	
private:
	bool					PerformTrain();
	bool					PerformTest();
	bool					LoadFeatureWeightFile();
	bool					InitWeight(PerceptronVector &ptd);
	bool					CreateIndexes();

// functii virtuale
	virtual bool			OnUpdateBest(PerceptronVector &pv,GML::Utils::AlgorithmResult &Result,bool &bestUpdated);
	virtual void			OnTestTerminateCondition(GML::Utils::AlgorithmResult &Result);
	virtual bool			OnInit();
	virtual bool			OnSaveData(char *fileName,GML::Utils::AlgorithmResult *Result=NULL);
	virtual bool			OnSaveBest(char *fileName,GML::Utils::AlgorithmResult *Result=NULL);

// functii virtuale pure
	virtual bool			PerformTrainIteration(UInt32 iteration)=0;
	virtual bool			PerformTestIteration(GML::Utils::AlgorithmResult &Result)=0;

protected:
		
	bool					Load(PerceptronVector &ptd,char *fileName);
	bool					Save(PerceptronVector &pv,char *fileName,GML::Utils::AlgorithmResult *result=NULL);
	bool					Train(PerceptronVector &pvTrain,PerceptronVector &pvTest,GML::Utils::Interval &interval,GML::ML::MLRecord &rec,GML::Utils::Indexes *indexes=NULL);
	bool					Test(PerceptronVector &pvTest,GML::Utils::Interval &range,GML::ML::MLRecord &rec,GML::Utils::AlgorithmResult &Result,GML::Utils::Indexes *indexes=NULL,bool resetResult=true,bool resetResultTime=true);

public:
	GenericPerceptron();

	bool					Init();
	void					OnExecute();
};


#endif
