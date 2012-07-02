#ifndef _FULL_CACHE_CONNECTOR_H
#define _FULL_CACHE_CONNECTOR_H

#include "gmllib.h"

using namespace GML;
using namespace GML::DB;
using namespace GML::ML;
using namespace GML::Utils;
						

class FullCacheConnector: public IConnector
{
private:
	double		*Records;
	double		*Labels;
	double		*Weights;
		

	bool		OnInitConnectionToDataBase();
public:
	FullCacheConnector();
	~FullCacheConnector();

	bool		GetRecord(MLRecord &record,UInt32 index,UInt32 recordMask=0 );
	bool		CreateMlRecord( MLRecord &record );
	bool		FreeMLRecord( MLRecord &record );
	bool		Close();
	bool		GetRecordLabel(double &label,UInt32 index);
	bool		GetRecordWeight(UInt32 index,double &weight);

	bool		Save(char *fileName);
	bool		Load(char *fileName);

};

#endif