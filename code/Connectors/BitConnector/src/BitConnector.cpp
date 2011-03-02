#include "BitConnector.h"

BitConnector::BitConnector()
{
	nrRecords = 0;
	Data = NULL;
	ObjectName = "BitConnector";
}

bool	BitConnector::OnInit()
{
	UInt32										tr,gr;
	GML::Utils::GTFVector<GML::DB::DBRecord>	VectPtr;
	UInt8										*cPoz;
	GML::DB::DBRecord							*rec;

	notifier->Info("BitConnect loading data");

	if (database->Connect()==false)
	{
		notifier->Error("could not connect to database");
		return false;
	}
	nrRecords = database->Select("*");
	if (nrRecords==0) 
	{
		notifier->Error("I received 0 records from the database");
		return false;
	}
	// aloca data :D
	if (((columns.nrFeatures+1)%8)==0)
		Align8Size = (columns.nrFeatures+1)/8;
	else
		Align8Size = ((columns.nrFeatures+1)/8)+1;

	if ((Data = new UInt8[nrRecords*Align8Size])==NULL)
	{
		notifier->Error("Unable to allocate %ud bytes for data indexes !",nrRecords*Align8Size);
		return false;
	}
	memset(Data,0,nrRecords*Align8Size);
	// sunt exact la inceput
	cPoz = Data;
	notifier->Info("BitConnect data (Records=%d,Features=%d,MemSize=%d,RecordsSize=%d)",nrRecords,columns.nrFeatures,nrRecords*Align8Size,Align8Size);



	for (tr=0;tr<nrRecords;tr++)
	{
		if (database->FetchNextRow(VectPtr)==false)
		{
			notifier->Error("Error reading #%d record !",tr);
			return false;
		}
		// pentru fiecare record pun valorile
		for (gr=0;gr<columns.nrFeatures;gr++)
		{
			if ((rec=VectPtr.GetPtrToObject(columns.indexFeature[gr]))==NULL)
			{
				notifier->Error("Unable to read record #%d",gr);
				return false;
			}
			if (rec->DoubleVal==1.0)
				cPoz[gr/8] |= (1<<(gr%8));
		}
		// pun si label-ul
		if ((rec=VectPtr.GetPtrToObject(columns.indexLabel))==NULL)
		{
			notifier->Error("Unable to read record #%d",columns.indexLabel);
			return false;
		}
		if (rec->DoubleVal==1.0)
			cPoz[columns.nrFeatures/8] |= (1<<(columns.nrFeatures%8));
		// trecem la urmatorul record
		cPoz+=Align8Size;
	}	
	// all ok , am incarcat datele
	return true;
}
bool	BitConnector::Close()
{
	return true;
}
bool	BitConnector::SetRecordInterval(UInt32 start, UInt32 end)
{
	return true;
}
bool	BitConnector::CreateMlRecord (GML::ML::MLRecord &record)
{
	record.FeatCount = columns.nrFeatures;	
	return ((record.Features = new double[columns.nrFeatures])!=NULL);
}
bool	BitConnector::GetRecord(GML::ML::MLRecord &record,UInt32 index)
{
	UInt8	*cPoz = &Data[index*Align8Size];
	UInt32	tr;

	if (index>=nrRecords)
		return false;

	for (tr=0;tr<columns.nrFeatures;tr++)
	{
		if ((cPoz[tr/8] & (1<<(tr%8)))!=0)
			record.Features[tr]=1.0;
		else
			record.Features[tr]=0;
	}
	// pun si label-ul
	if ((cPoz[columns.nrFeatures/8] & (1<<(columns.nrFeatures%8)))!=0)
		record.Label = 1.0;
	else
		record.Label = -1.0;

	return true;
}
bool	BitConnector::GetRecordLabel(double &Label,UInt32 index)
{
	UInt8	*cPoz = &Data[index*Align8Size];

	if (index>=nrRecords)
		return false;

	// pun si label-ul
	if ((cPoz[columns.nrFeatures/8] & (1<<(columns.nrFeatures%8)))!=0)
		Label = 1.0;
	else
		Label = -1.0;

	return true;
}
bool	BitConnector::FreeMLRecord(GML::ML::MLRecord &record)
{
	if (record.Features!=NULL)
	{
		delete record.Features;
		record.Features = NULL;
	}
	return true;
}
UInt32	BitConnector::GetFeatureCount()
{
	return columns.nrFeatures;
}
UInt32	BitConnector::GetRecordCount()
{
	return nrRecords;
}
UInt32	BitConnector::GetTotalRecordCount()
{
	return nrRecords;
}
