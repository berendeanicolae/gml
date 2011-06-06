#ifndef __PCA_H__
#define __PCA_H__

#include "gmllib.h"

class PCA: public GML::Algorithm::IMLAlgorithm
{
	GML::Utils::GTFVector<double>	p,
									t;

	GML::ML::MLRecord		ob;
	double					thereshold;
	UInt32					pca_nr,
							rc,
							fc;
	GML::Utils::GString		filePath;

	GML::Utils::FixMatrix<double> matrix;

public:
	PCA();
	bool	Init();
	void	OnExecute();
	bool					OnInitThreadData(GML::Algorithm::MLThreadData &thData);
	void					OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
};

#endif