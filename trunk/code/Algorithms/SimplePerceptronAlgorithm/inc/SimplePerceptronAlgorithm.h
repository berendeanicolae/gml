#ifndef __SIMPLE_PERCEPTRON_ALG__
#define __SIMPLE_PERCEPTRON_ALG__

#include "GMLLib.h"

class SimplePerceptronAlgorithm: public GML::Algorithm::IAlgorithm
{
	GML::Utils::INotify		*notif;
	GML::DB::IDataBase		*db;
	GML::ML::IConector		*con;
	GML::ML::MLRecord		rec;
	double					*weight;


	GML::Utils::GString		dbName,conString,conectorString,notifString;
	double					learningRate;
	UInt32					maxIteratii;

protected:
	void	Train();
	void	Test();

public:
	SimplePerceptronAlgorithm();
	bool	SetConfiguration(GML::Utils::AttributeList &config);
	bool	GetConfiguration(GML::Utils::AttributeList &config);
	bool	Init();
	void	Execute(UInt32 command);
};


#endif
