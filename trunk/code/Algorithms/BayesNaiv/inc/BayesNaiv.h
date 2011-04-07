#include "gmllib.h"
#include <math.h>

class BayesNaiv: public GML::Algorithm::IAlgorithm
{	
	enum {
		COMMAND_NONE = 0,
		COMMAND_TRAIN,
		COMMAND_TEST
	};



	GML::DB::IDataBase		*db;
	GML::ML::IConnector		*con;

	GML::Utils::GString		strDB;
	GML::Utils::GString		strConector;
	GML::Utils::GString		strNotificator;

public:
	BayesNaiv();

	bool	Init();

	bool	PerformTrain();
	bool	PerformTest();

	void	OnExecute();
};