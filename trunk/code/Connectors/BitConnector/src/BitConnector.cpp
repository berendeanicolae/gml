#include "BitConnector.h"

BitConnector::BitConnector()
{
	nrRecords = nrFeatures = 0;
	Data = NULL;
	FeaturesIndexs = NULL;

}
bool	BitConnector::UpdateColumnInfos(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)
{
	UInt32				tr,value;
	GML::DB::DBRecord	*rec;

	nrFeatures = 0;
	LabelIndex = (UInt32)-1;
	for (tr=0;tr<VectPtr.Len();tr++)
	{
		if ((rec=VectPtr.GetPtrToObject(tr))==NULL)
		{
			notifier->Error("Unable to read record #%d",tr);
			return false;
		}
		if (GML::Utils::GString::Equals(rec->Name,"Label",true))
			LabelIndex = tr;
		if (GML::Utils::GString::StartsWith(rec->Name,"Feat_",true))
			nrFeatures++;
	}
	if ((nrFeatures==0) || (LabelIndex==(UInt32)-1))
	{
		notifier->Error("Missing Feature or Label column");
		return false;
	}
	// aloc indexii
	if ((FeaturesIndexs = new UInt32[nrFeatures])==NULL)
	{
		notifier->Error("Unable to alloc %d features indexes ",nrFeatures);
		return false;
	}
	// pun o valoare default
	for (tr=0;tr<nrFeatures;tr++)
	{
		FeaturesIndexs[tr] = (UInt32)-1;
	}
	// setez si indexii
	for (tr=0;tr<VectPtr.Len();tr++)
	{
		if ((rec=VectPtr.GetPtrToObject(tr))==NULL)
		{
			notifier->Error("Unable to read record #%d",tr);
			return false;
		}
		if (GML::Utils::GString::StartsWith(rec->Name,"Feat_",true))
		{
			if (GML::Utils::GString::ConvertToUInt32(&rec->Name[5],&value)==false)
			{
				notifier->Error("Invalid numeric format on column: %s",rec->Name);
				return false;
			}
			if (value>=nrFeatures)
			{
				notifier->Error("Out of the bounds feature ([0..%d)) -> %s",nrFeatures,rec->Name);
				return false;
			}
			FeaturesIndexs[value] = tr;
		}
	}
	// verific sa fi fost setati toti featureii
	for (tr=0;tr<nrFeatures;tr++)
	{
		if (FeaturesIndexs[tr]==(UInt32)-1)
		{
			notifier->Error("Feature #d is not present !",tr);
			return false;
		}
	}
	// totul e ok -> 
	return true;
}
bool	BitConnector::OnInit()
{
	UInt32										tr,gr;
	GML::Utils::GTFVector<GML::DB::DBRecord>	VectPtr;
	UInt8										*cPoz;
	GML::DB::DBRecord							*rec;

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
	for (tr=0;tr<nrRecords;tr++)
	{
		if (database->FetchNextRow(VectPtr)==false)
		{
			notifier->Error("Error reading #%d record !",tr);
			return false;
		}
		// daca e primul record fac si alocarile
		if (tr==0)
		{
			if (UpdateColumnInfos(VectPtr)==false)
				return false;
			if (((nrFeatures+1)%8)==0)
				Align8Size = (nrFeatures+1)/8;
			else
				Align8Size = ((nrFeatures+1)/8)+1;
			if ((Data = new UInt8[nrRecords*Align8Size])==NULL)
			{
				notifier->Error("Unable to allocate %ud bytes for data indexes !",nrRecords*Align8Size);
				return false;
			}
			memset(Data,0,nrRecords*Align8Size);
			// sunt exact la inceput
			cPoz = Data;
			notifier->Info("BitConect data (Records=%d,Features=%d,MemSize=%d,RecordsSize=%d)",nrRecords,nrFeatures,nrRecords*Align8Size,Align8Size);
		}
		// pentru fiecare record pun valorile
		for (gr=0;gr<nrFeatures;gr++)
		{
			if ((rec=VectPtr.GetPtrToObject(FeaturesIndexs[gr]))==NULL)
			{
				notifier->Error("Unable to read record #%d",gr);
				return false;
			}
			if (rec->DoubleVal==1.0)
				cPoz[gr/8] |= (1<<(gr%8));
		}
		// pun si label-ul
		if ((rec=VectPtr.GetPtrToObject(LabelIndex))==NULL)
		{
			notifier->Error("Unable to read record #%d",LabelIndex);
			return false;
		}
		if (rec->DoubleVal==1.0)
			cPoz[nrFeatures/8] |= (1<<(nrFeatures%8));
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
	record.FeatCount = nrFeatures;
	return ((record.Features = new double[nrFeatures])!=NULL);
}
bool	BitConnector::GetRecord(GML::ML::MLRecord &record,UInt32 index)
{
	UInt8	*cPoz = &Data[index*Align8Size];
	UInt32	tr;

	for (tr=0;tr<nrFeatures;tr++)
	{
		if ((cPoz[tr/8] & (1<<(tr%8)))!=0)
			record.Features[tr]=1.0;
		else
			record.Features[tr]=0;
	}
	// pun si label-ul
	if ((cPoz[nrFeatures/8] & (1<<(nrFeatures%8)))!=0)
		record.Label = 1.0;
	else
		record.Label = -1.0;

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
	return nrFeatures;
}
UInt32	BitConnector::GetRecordCount()
{
	return nrRecords;
}
UInt32	BitConnector::GetTotalRecordCount()
{
	return nrRecords;
}
