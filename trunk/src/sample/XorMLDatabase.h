#ifndef __XORMLDATABASE_H
#define __XORMLDATABASE_H

#include "gml.h"

class XorMlDatabase : public IMLDatabase
{
private:


public:

	XorMlDatabase();
	~XorMlDatabase();

	virtual UInt32 GetRecordCount();

	virtual UInt32 GetFeatureCount();

	virtual bool Init(INotifier &Notifier,IDatabase &Database);

	virtual bool GetRecord( MLRecord &record,UInt32 index );

	virtual bool SetRecordInterval( UInt32 start, UInt32 end );

	virtual bool FreeMLRecord( MLRecord &record );

	virtual bool Close();

	virtual UInt32 GetTotalRecordCount();

	virtual MLRecord * CreateMlRecord ();

};

#endif