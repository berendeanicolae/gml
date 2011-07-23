#include "IndexBitConnector.h"

#define CACHE_SIG_NAME "IndexBitConnectorV2"

IndexBitConnector::IndexBitConnector()
{
	Data = NULL;
	Indexes = NULL;
	ObjectName = "IndexBitConnector";

	AddTwoClassLabelProperties();
	AddCacheProperties();
	AddStoreProperties();
}

bool	IndexBitConnector::AllocMemory(UInt64 memory)
{
	GML::Utils::GString	temp;
	// aloca data :D
	temp.Set("");
	temp.AddFormatedEx("[%{str}] -> Compressed method : %{uint32,dec} (Memory: %{uint64,G3,dec} bytes)",ObjectName,Method,memory); 
	notifier->Info("%s",temp.GetText());

	if ((Data = new UInt8[memory])==NULL)
	{
		notifier->Error("[%s] -> Unable to allocate %ud bytes for data !",ObjectName,memory);
		return false;
	}
	if ((Indexes = new UInt64[nrRecords])==NULL)
	{
		notifier->Error("[%s] -> Unable to allocate %ud bytes for index data !",ObjectName,nrRecords);
		return false;
	}
	memset(Data,0,(UInt32)memory);
	memset(Indexes,0,nrRecords*sizeof(UInt32));

	if (Labels.Create(nrRecords)==false)
	{
		notifier->Error("[%s] -> Unable to alloc memory for label ... !",ObjectName);
		return false;
	}
	Labels.SetAll(false);
	if (StoreRecordHash)
	{
		if (Hashes.Create(nrRecords)==false)
		{
			notifier->Error("[%s] -> Unable to allocate %ud bytes for hashes !",ObjectName,nrRecords*sizeof(GML::DB::RecordHash));
			return false;
		}
	}
	return true;
}
void	IndexBitConnector::Update(IndexBitCounter &ibt,UInt32 index)
{
	ibt.countInt32++;
	if (index<256)
		ibt.countInt8++;
	if (index<=0xFFFF)
		ibt.countInt16++;	
	if (index<255)
		ibt.count254BaseInt8++;
	if ((index>=255) && (index<=255+255))
		ibt.count254BaseInt16++;
	if (index<253)
		ibt.count253BaseInt8++;
	if ((index>=254) && (index<=254+255+255))
		ibt.count253BaseInt16++;
	if (index>ibt.maxIndex)
		ibt.maxIndex = index;
}
bool	IndexBitConnector::AddIndex(UInt32 index,UInt64 &poz)
{
	switch (Method)
	{
		case METHOD_INT8_INDEX:
			if (poz+1>MemToAlloc)
				return false;
			Data[poz] = (UInt8)index;
			poz++;
			return true;
		case METHOD_INT16_INDEX:
			if (poz+sizeof(UInt16)>MemToAlloc)
				return false;
			*(UInt16 *)&Data[poz] = (UInt16)index;
			poz+=sizeof(UInt16);
			return true;
		case METHOD_INT32_INDEX:
			if (poz+sizeof(UInt32)>MemToAlloc)
				return false;
			*(UInt32 *)&Data[poz] = (UInt32)index;
			poz+=sizeof(UInt32);
			return true;
		case METHOD_254_BASE_INDEX:
			if (index<255)
			{
				if (poz+1>MemToAlloc)
					return false;
				Data[poz] = (UInt8)index;
				poz++;
			} else {
				if (poz+2>MemToAlloc)
					return false;
				Data[poz++] = 0xFF;
				Data[poz++] = (UInt8)(index-0xFF);
			}
			return true;
	};
	return false;
}
void	IndexBitConnector::ComputeMemory(IndexBitCounter &ibt,UInt64 &memory)
{
	if (ibt.maxIndex <256)
	{
		Method = METHOD_INT8_INDEX;
		memory = (UInt64)ibt.countInt8;
		return;
	}
	if (ibt.maxIndex <=255+255)
	{
		Method = METHOD_254_BASE_INDEX;
		memory = (UInt64)ibt.count254BaseInt8+(UInt64)ibt.count254BaseInt16 * sizeof(UInt16);
		return;
	}
	if (ibt.maxIndex <=254+255+255)
	{
		Method = METHOD_253_BASE_INDEX;
		memory = (UInt64)ibt.count253BaseInt8+(UInt64)ibt.count253BaseInt16 * sizeof(UInt16);
		return;
	}
	if (ibt.maxIndex <= 0xFFFF)
	{
		Method = METHOD_INT16_INDEX;
		memory = (UInt64)ibt.countInt16 * sizeof(UInt16);
		return;
	}
	Method = METHOD_INT32_INDEX;
	memory = (UInt64)ibt.countInt32 * sizeof(UInt32);
}
bool	IndexBitConnector::OnInitConnectionToConnector()
{
	UInt32										tr,gr,recMask;	
	UInt64										cIndex;
	UInt8										*cPoz;
	GML::ML::MLRecord							cRec;
	GML::Utils::GString							temp;
	IndexBitCounter								ibc;
	bool										Label;

	columns.nrFeatures = conector->GetFeatureCount();
	nrRecords = conector->GetRecordCount();
	if (UpdateFeaturesNameFromConnector()==false)
		return false;
	if (nrRecords==0) 
	{
		notifier->Error("[%s] -> I received 0 records from the parent connector",ObjectName);
		return false;
	}
	if (conector->CreateMlRecord(cRec)==false)
	{
		notifier->Error("[%s] -> Unable to crea MLRecord ",ObjectName);
		return false;
	}
	memset(&ibc,0,sizeof(ibc));
	notifier->StartProcent("[%s] -> Analizing DataBase : ",ObjectName);
	for (tr=0;tr<nrRecords;tr++)
	{
		if ((tr % 10000)==0)
			notifier->SetProcent((double)tr,(double)nrRecords);
		
		if (conector->GetRecord(cRec,tr)==false)		
		{
			notifier->Error("[%s] -> Error reading #%d record from parent connector!",ObjectName,tr);
			return false;
		}
		// pentru fiecare record pun valorile
		for (gr=0;gr<columns.nrFeatures;gr++)
			if (cRec.Features[gr]!=0.0)
				Update(ibc,gr);	
	}	
	notifier->EndProcent();	
	ComputeMemory(ibc,MemToAlloc);
	if (AllocMemory(MemToAlloc)==false)
		return false;
	
	// sunt exact la inceput
	cPoz = Data;
	recMask = 0;
	if (StoreRecordHash)
		recMask |= GML::ML::ConnectorFlags::STORE_HASH;

	notifier->StartProcent("[%s] -> Loading Data : ",ObjectName);
	cIndex = 0;
	for (tr=0;tr<nrRecords;tr++)
	{
		Indexes[tr] = cIndex;
		if ((tr % 10000)==0)
			notifier->SetProcent((double)tr,(double)nrRecords);
		
		if (conector->GetRecord(cRec,tr,recMask)==false)		
		{
			notifier->Error("[%s] -> Error reading #%d record from parent connector!",ObjectName,tr);
			return false;
		}
		// pentru fiecare record pun valorile
		for (gr=0;gr<columns.nrFeatures;gr++)
		{
			if (cRec.Features[gr]!=0.0)
			{
				if (AddIndex(gr,cIndex)==false)
				{
					temp.Set("");
					temp.AddFormatedEx("[%{str}] -> Unable to add Index to list : Records:%{uint32,dec} , Feature:%{uint32,dec} , Poz:%{uint64,dec,G3} , Alloc:%{uint64,dec,G3}",ObjectName,tr,gr,cIndex,MemToAlloc);
					notifier->Error("%s",temp.GetText());
					return false;
				}
			}
		}
		// pun si label-ul
		if (UpdateTwoClassLabelValue(cRec.Label,Label)==false)
			return false;
		if (Labels.Set(tr,Label)==false)
		{
			notifier->Error("[%s] -> Unable to set label for record #%d",tr);
			return false;
		}
		// adaug si Hash-ul
		if (StoreRecordHash)
		{
			if (Hashes.PushByRef(cRec.Hash)==false)
			{
				notifier->Error("[%s] -> Unable to save Hash with id %d",ObjectName,tr);
				return false;
			}
		}
		// trecem la urmatorul record
	}	
	notifier->EndProcent();
	// all ok , am incarcat datele
	if (cIndex!=MemToAlloc)
	{
			notifier->Error("[%s] -> Internal Error computing compressed size ... (cIndex=%d,MemToAlloc=%d)",ObjectName,(UInt32)cIndex,(UInt32)MemToAlloc);
			return false;
	}
	// all ok , am incarcat datele
	dataMemorySize = (UInt64)nrRecords * sizeof(UInt64) + MemToAlloc+Labels.GetAllocated();
	conector->FreeMLRecord(cRec);
	return true;

}
bool	IndexBitConnector::OnInitConnectionToDataBase()
{
	UInt32										tr,gr,colIndex;
	UInt64										cIndex;
	GML::Utils::GTFVector<GML::DB::DBRecord>	VectPtr;
	GML::Utils::GString							temp;
	GML::DB::RecordHash							cHash;
	double										cValue;
	IndexBitCounter								ibc;
	bool										Label;

	memset(&ibc,0,sizeof(ibc));
	notifier->StartProcent("[%s] -> Analizing DataBase : ",ObjectName);
	for (tr=0;tr<nrRecords;tr++)
	{
		if (database->ReadNextRecord(VectPtr)==false)
		{
			notifier->Error("[%s] -> Unable to read #d record from database!",ObjectName,tr);
			return false;
		}
		// pentru fiecare record pun valorile
		for (gr=0;gr<columns.nrFeatures;gr++)
		{
			colIndex = columns.indexFeature[gr];
			if (UpdateDoubleValue(VectPtr,colIndex,cValue)==false)
				return false;
			if (cValue!=0.0)
				Update(ibc,gr);				
		}
		if ((tr % 10000)==0)
			notifier->SetProcent(tr,nrRecords);
	}	
	notifier->EndProcent();
	notifier->Info("[%s] -> Total number of features: %d",ObjectName,ibc.countInt32);
	ComputeMemory(ibc,MemToAlloc);
	if (AllocMemory(MemToAlloc)==false)
		return false;
	if (database->BeginIteration()==false)
	{
		notifier->Error("[%s] -> Error on DataBase::BeginIteration()",ObjectName);
		return false;
	}
	
	cIndex = 0;
	notifier->StartProcent("[%s] -> Loading DataBase : ",ObjectName);

	for (tr=0;tr<nrRecords;tr++)
	{
		Indexes[tr] = cIndex;
		if (database->ReadNextRecord(VectPtr)==false)
		{
			notifier->Error("[%s] -> Unable to read #d record from database!",ObjectName,tr);
			return false;
		}
		// pentru fiecare record pun valorile
		for (gr=0;gr<columns.nrFeatures;gr++)
		{
			colIndex = columns.indexFeature[gr];
			if (UpdateDoubleValue(VectPtr,colIndex,cValue)==false)
				return false;
			if (cValue!=0.0)				
			{
				if (AddIndex(gr,cIndex)==false)
				{
					temp.Set("");
					temp.AddFormatedEx("[%{str}] -> Unable to add Index to list : Records:%{uint32,dec} , Feature:%{uint32,dec} , Poz:%{uint64,dec,G3} , Alloc:%{uint64,dec,G3}",ObjectName,tr,gr,cIndex,MemToAlloc);
					notifier->Error("%s",temp.GetText());
					return false;
				}
			}
		}
		// pun si label-ul
		if (UpdateDoubleValue(VectPtr,columns.indexLabel,cValue)==false)
			return false;
		if (UpdateTwoClassLabelValue(cValue,Label)==false)
			return false;
		if (Labels.Set(tr,Label)==false)
		{
			notifier->Error("[%s] -> Unable to set label for record #%d",ObjectName,tr);
			return false;
		}
		// adaug si Hash-ul
		if (StoreRecordHash)
		{
			if (UpdateHashValue(VectPtr,columns.indexHash,cHash)==false)
				return false;
			if (Hashes.PushByRef(cHash)==false)
			{
				notifier->Error("[%s] -> Unable to save Hash with id %d",ObjectName,tr);
				return false;
			}
		}
		// trecem la urmatorul record		
		if ((tr % 10000)==0)
			notifier->SetProcent(tr,nrRecords);
	}	
	notifier->EndProcent();
	if (cIndex!=MemToAlloc)
	{
			notifier->Error("[%s] -> Internal Error computing compressed size ... (cIndex=%d,MemToAlloc=%d)",ObjectName,(UInt32)cIndex,(UInt32)MemToAlloc);
			return false;
	}
	// all ok , am incarcat datele
	dataMemorySize = (UInt64)nrRecords * sizeof(UInt64) + MemToAlloc+Labels.GetAllocated();
	return true;
}

bool	IndexBitConnector::Close()
{
	if (Data!=NULL)
		delete Data;
	if (Indexes!=NULL)
		delete Indexes;
	Data = NULL;
	Indexes = NULL;
	return true;
}
bool	IndexBitConnector::Save(char *fileName)
{
	IndexBitConnectorHeader		h;

	while (true)
	{
		if (CreateCacheFile(fileName,CACHE_SIG_NAME,&h,sizeof(h),Method)==false)
			break;
		h.MemToAlloc = MemToAlloc;
		if (file.Write(&h,sizeof(h))==false)
			break;
		if (file.Write(Data,MemToAlloc)==false)
			break;
		if (file.Write(Indexes,sizeof(UInt64)*((UInt64)nrRecords))==false)
			break;
		if (file.Write(Labels.GetData(),Labels.GetAllocated())==false)
			break;
		if (SaveRecordHashesAndFeatureNames()==false)
			break;
		CloseCacheFile();
		return true;
	}
	notifier->Error("[%s] Unable to write into %s",ObjectName,fileName);
	CloseCacheFile();
	DeleteFileA(fileName);
	return false;
}
bool	IndexBitConnector::Load(char *fileName)
{
	IndexBitConnectorHeader	h;
	while (true)
	{
		if (OpeanCacheFile(fileName,CACHE_SIG_NAME,&h,sizeof(h))==false)
			break;
		if (Data!=NULL)
			delete Data;
		if (Indexes!=NULL)
			delete Indexes;
		Data = NULL;
		Indexes = NULL;
		MemToAlloc = h.MemToAlloc;
		Method = h.Flags; 
	
		if (AllocMemory(h.MemToAlloc)==false)
		{
			notifier->Error("[%s] -> Unable to allocate space for cache initialization",ObjectName);
			break;
		}
		if (file.Read(Data,MemToAlloc)==false)
			break;
		if (file.Read(Indexes,sizeof(UInt64)*((UInt64)nrRecords))==false)
			break;
		if (file.Read(Labels.GetData(),Labels.GetAllocated())==false)
			break;
		if (LoadRecordHashesAndFeatureNames(&h)==false)
			break;
		CloseCacheFile();
		dataMemorySize = (UInt64)nrRecords * sizeof(UInt64) + MemToAlloc+Labels.GetAllocated();
		return true;		
	}
	ClearColumnIndexes();
	CloseCacheFile();
	notifier->Error("[%s] -> Error read data from %s",ObjectName,fileName);
	return false;
}

bool	IndexBitConnector::CreateMlRecord (GML::ML::MLRecord &record)
{
	record.FeatCount = columns.nrFeatures;	
	return ((record.Features = new double[columns.nrFeatures])!=NULL);
}
bool	IndexBitConnector::GetRecord(GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask)
{
	UInt8	*cPoz,*end;
	UInt32	sz,indexFeat;

	if (index>=nrRecords)
		return false;
	cPoz = &Data[Indexes[index]];
	if (index+1==nrRecords)
	{
		sz = (UInt32)MemToAlloc -Indexes[index];
	} else {
		sz = Indexes[index+1]-Indexes[index];
	}
	end = cPoz+sz;
	memset(record.Features,0,sizeof(double)*columns.nrFeatures);
	while (cPoz<end)
	{
		switch (Method)
		{
			case METHOD_INT8_INDEX:
				indexFeat = (UInt32)(*cPoz);
				cPoz++;
				break;
			case METHOD_INT16_INDEX:
				indexFeat = *((UInt16*)cPoz);
				cPoz+=sizeof(UInt16);
				break;
			case METHOD_INT32_INDEX:
				indexFeat = *((UInt32*)cPoz);
				cPoz+=sizeof(UInt32);
				break;
			case METHOD_254_BASE_INDEX:
				if ((*cPoz)<255)
				{
					indexFeat = (*cPoz);
					cPoz++;
				} else {
					indexFeat = 255+cPoz[1];
					cPoz+=2;
				}
				break;
			default:
				return false;
		};
		if (indexFeat>=columns.nrFeatures)
		{
			notifier->Error("[%s] -> Invalid index for feature : %d ",ObjectName,indexFeat);
			return false;
		}
		record.Features[indexFeat] = 1.0;
	}


	// pun si label-ul
	if (Labels.Get(index))
		record.Label = OutLabelPositive;
	else
		record.Label = OutLabelNegative;

	if (recordMask & GML::ML::ConnectorFlags::STORE_HASH)
	{
		if (StoreRecordHash)
			record.Hash.Copy(*Hashes.GetPtrToObject(index));
		else
			record.Hash.Reset();
	}

	return true;
}
bool	IndexBitConnector::GetRecordLabel(double &Label,UInt32 index)
{
	if (index>=nrRecords)
		return false;

	// pun si label-ul
	if (Labels.Get(index))
		Label = OutLabelPositive;
	else
		Label = OutLabelNegative;

	return true;
}
bool	IndexBitConnector::FreeMLRecord(GML::ML::MLRecord &record)
{
	if (record.Features!=NULL)
	{
		delete record.Features;
		record.Features = NULL;
	}
	return true;
}
