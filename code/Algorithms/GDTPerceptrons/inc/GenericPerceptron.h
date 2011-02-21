#ifndef __GENERIC_PERCEPTRON__
#define __GENERIC_PERCEPTRON__

#include "gmllib.h"

class GenericPerceptron : public GML::Algorithm::IAlgorithm
{
	GML::DB::IDataBase		*db;
	GML::ML::IConector		*con;

	// proprietati
	GML::Utils::GString		Name;
	GML::Utils::GString		Conector;
	GML::Utils::GString		DataBase;
	GML::Utils::GString		Notifier;

	// proprietati de training
	double					learningRate;
	bool					useWeight;
	UInt32					testAfterIterations;


	// proprietati de oprire
	double					minimAcc;
	double					minimSe;
	double					minimSp;
	UInt32					maxIterations;

public:
	GenericPerceptron();

	bool					Init();
};


#endif
