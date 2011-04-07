
#include "gmllib.h"

class Projectron: public GML::Algorithm::IAlgorithm
{
	enum
	{
		POLYNOMIAL_KER = 0,
		SIGMOYD_KER
	};

	enum
	{
		COMMAND_NONE = 0,
		COMMAND_TRAIN,
		COMMAND_TEST
	};

	//Hilbert Space
	GML::Utils::Indexes		St;
	//K matrix
	GML::Utils::Matrix		K;
	GML::DB::IDataBase		*db;
	GML::ML::IConnector		*con;

	GML::Utils::GString		strDB;
	GML::Utils::GString		strConector;
	GML::Utils::GString		strNotificator;

	UInt32					kernelFunction;

	
public:
	Projectron();
	bool					PerformTrain();
	bool					PerformTest();
	bool					Init();
	void					OnExecute();
};