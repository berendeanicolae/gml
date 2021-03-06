#include "CompressedBitConnector.h"

#define CACHE_SIG_NAME  "CompressedBitConnectorV1"

#define COMPUTE_INDEX_FEAT(base) \
				if ((*cPoz)<=base) \
				{ \
					indexFeat = Last + (*cPoz); \
					cPoz++; \
				} else { \
					indexFeat = Last + (UInt32)((UInt32)base+1+(UInt32)((*cPoz)-(base+1))*256+(UInt32)(cPoz[1])); \
					cPoz+=2; \
				}
#define ADD_INDEX_FEAT(base) \
			if (index<=base) \
			{ \
				if (poz+1>MemToAlloc) \
					return false; \
				Data[poz] = (UInt8)index; \
				poz++; \
			} else { \
				if (poz+2>MemToAlloc) \
					return false; \
				Data[poz++] = (UInt8)(base+1) + (UInt8)((index-(base+1))>>8); \
				Data[poz++] = (UInt8)((index-(base+1)) & 0xFF); \
			} \
			return true; 
#define UPDATE_INDEX_FEAT(base,counterLo,counterHi) \
			if (index<=base) { counterLo++; } \
			if ((index>base) && (index<=((256-(UInt32)base)*255))) { counterHi++; }

#define COMPUTE_MEMORY(base,maxValue,methodID,counterLo,counterHi) \
			if (maxValue<=(UInt32)((256-(UInt32)base)*255)) \
			{ \
				Method = methodID; \
				memory = (UInt64)counterLo + ((UInt64)counterHi)*2; \
				return; \
			}

CompressedBitConnector::CompressedBitConnector()
{
	Data = NULL;
	Indexes = NULL;
	ObjectName = "CompressedBitConnector";
	AddTwoClassLabelProperties();
	AddCacheProperties();
	AddStoreProperties();	
	
	LinkPropertyToUInt32("CacheMemorySize",CacheMemory,0,"Memory allocated to cache the records.\nIf 0 all the records will be loaded in memory.\nWorks only if the records are loaded from a cache!");
}
bool	CompressedBitConnector::AllocMemory(UInt64 memory)
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
void	CompressedBitConnector::Update(IndexBitCounter &ibt,UInt32 value)
{
	UInt32	index = value-ibt.Last;
	
	ibt.countInt32++;
	ibt.Last = value;
	if (index<128)
		ibt.count7Bit++;
	if (index<(1<<15))
		ibt.count15Bit++;	
	if (index<256)
		ibt.countInt8++;
	if (index==255)
		ibt.countValue255++;
	if (index<=0xFFFF)
		ibt.countInt16++;	
	UPDATE_INDEX_FEAT(254,ibt.count254BaseLo,ibt.count254BaseHi);
	UPDATE_INDEX_FEAT(253,ibt.count253BaseLo,ibt.count253BaseHi);
	UPDATE_INDEX_FEAT(252,ibt.count252BaseLo,ibt.count252BaseHi);
	UPDATE_INDEX_FEAT(251,ibt.count251BaseLo,ibt.count251BaseHi);
	UPDATE_INDEX_FEAT(250,ibt.count250BaseLo,ibt.count250BaseHi);
		
	if (index>ibt.maxIndex)
		ibt.maxIndex = index;
}
bool	CompressedBitConnector::AddIndex(UInt32 index,UInt64 &poz)
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
			ADD_INDEX_FEAT(254);
			return true;
		case METHOD_253_BASE_INDEX:
			ADD_INDEX_FEAT(253);
			return true;
		case METHOD_252_BASE_INDEX:
			ADD_INDEX_FEAT(252);
			return true;
		case METHOD_251_BASE_INDEX:
			ADD_INDEX_FEAT(251);
			return true;
		case METHOD_250_BASE_INDEX:
			ADD_INDEX_FEAT(250);
			return true;
		case METHOD_255_BASE_INDEX:
			if (index<255)
			{
				if (poz+1>MemToAlloc)
					return false;
				Data[poz] = (UInt8)index;
				poz++;
			} else {
				if (poz+3>MemToAlloc)
					return false;
				Data[poz++] = 0xFF;
				*(UInt16 *)&Data[poz] = (UInt16)index;
				poz+=sizeof(UInt16);
			}
			return true;
		case METHOD_INT15_EXTEND_INDEX:
			if (index<128)
			{
				if (poz+1>MemToAlloc)
					return false;
				Data[poz] = (UInt8)index;
				poz++;
			} else {
				if (poz+2>MemToAlloc)
					return false;
				Data[poz++] = (index & 127)|128;
				Data[poz++] = (UInt8)((index>>7) & 0xFF);				
			}
			return true;			
	};
	return false;
}
void	CompressedBitConnector::ComputeMemory(IndexBitCounter &ibt,UInt64 &memory)
{
	UInt64	tmp;
	
	if (ibt.maxIndex <256)
	{
		Method = METHOD_INT8_INDEX;
		memory = (UInt64)ibt.countInt8;
		return;
	}
	COMPUTE_MEMORY(254,ibt.maxIndex,METHOD_254_BASE_INDEX,ibt.count254BaseLo,ibt.count254BaseHi);
	COMPUTE_MEMORY(253,ibt.maxIndex,METHOD_253_BASE_INDEX,ibt.count253BaseLo,ibt.count253BaseHi);
	COMPUTE_MEMORY(252,ibt.maxIndex,METHOD_252_BASE_INDEX,ibt.count252BaseLo,ibt.count252BaseHi);
	COMPUTE_MEMORY(251,ibt.maxIndex,METHOD_251_BASE_INDEX,ibt.count251BaseLo,ibt.count251BaseHi);
	COMPUTE_MEMORY(250,ibt.maxIndex,METHOD_250_BASE_INDEX,ibt.count250BaseLo,ibt.count250BaseHi);
	
	if (ibt.maxIndex <= 0xFFFF)
	{
		Method = METHOD_INT16_INDEX;
		memory = (UInt64)ibt.countInt16 * sizeof(UInt16);
		tmp = ((UInt64)(ibt.countInt16-ibt.count254BaseLo))*3+ibt.count254BaseLo;
		if (tmp<memory)
		{
			Method = METHOD_255_BASE_INDEX;
			memory = tmp;
		}
		if ((ibt.maxIndex < (1<<15)) && (ibt.count7Bit>=0))
		{			
			tmp = (UInt64)ibt.count7Bit + ((UInt64)(ibt.count15Bit-ibt.count7Bit))*sizeof(UInt16);
			if (tmp<memory)
			{
				memory = tmp;
				Method = METHOD_INT15_EXTEND_INDEX;
			}
		}		
		return;
	}
	Method = METHOD_INT32_INDEX;
	memory = (UInt64)ibt.countInt32 * sizeof(UInt32);
}
bool	CompressedBitConnector::OnInitConnectionToConnector()
{
	UInt32										tr,gr,recMask,Last;	
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
		ibc.Last = 0;
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
		Last = 0;
		for (gr=0;gr<columns.nrFeatures;gr++)
		{
			if (cRec.Features[gr]!=0.0)
			{
				if (AddIndex(gr-Last,cIndex)==false)
				{
					temp.Set("");
					temp.AddFormatedEx("[%{str}] -> Unable to add Index to list : Records:%{uint32,dec} , Feature:%{uint32,dec} , Poz:%{uint64,dec,G3} , Alloc:%{uint64,dec,G3}",ObjectName,tr,gr,cIndex,MemToAlloc);
					notifier->Error("%s",temp.GetText());
					return false;
				}
				Last = gr;
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
	if (DuplicateRecordWeights(conector)==false)
		return false;	
	// all ok , am incarcat datele
	dataMemorySize = (UInt64)nrRecords * (UInt64)sizeof(UInt64) + MemToAlloc+Labels.GetAllocated();
	conector->FreeMLRecord(cRec);
	return true;

}
bool	CompressedBitConnector::OnInitConnectionToDataBase()
{
	UInt32										tr,gr,colIndex,Last;
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
		ibc.Last = 0;
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
		Last = 0;
		for (gr=0;gr<columns.nrFeatures;gr++)
		{
			colIndex = columns.indexFeature[gr];
			if (UpdateDoubleValue(VectPtr,colIndex,cValue)==false)
				return false;
			if (cValue!=0.0)				
			{
				if (AddIndex(gr-Last,cIndex)==false)
				{
					temp.Set("");
					temp.AddFormatedEx("[%{str}] -> Unable to add Index to list : Records:%{uint32,dec} , Feature:%{uint32,dec} , Poz:%{uint64,dec,G3} , Alloc:%{uint64,dec,G3}",ObjectName,tr,gr,cIndex,MemToAlloc);
					notifier->Error("%s",temp.GetText());
					return false;
				}
				Last = gr;
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
	dataMemorySize = (UInt64)nrRecords * (UInt64)sizeof(UInt64) + MemToAlloc+Labels.GetAllocated();
	return true;
}

bool	CompressedBitConnector::Close()
{
	if (Data!=NULL)
		delete Data;
	if (Indexes!=NULL)
		delete Indexes;
	Data = NULL;
	Indexes = NULL;
	return true;
}
bool	CompressedBitConnector::Save(char *fileName)
{
	CompressedBitConnectorHeader		h;

	while (true)
	{
		if (CreateCacheFile(fileName,CACHE_SIG_NAME,&h,sizeof(h),Method)==false)
			break;
		h.MemToAlloc = MemToAlloc;
		if (file.Write(&h,sizeof(h))==false)
			break;
		if (file.Write(Data,(UInt64)MemToAlloc)==false)
			break;
		if (file.Write(Indexes,sizeof(UInt64)*(UInt64)nrRecords)==false)
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
bool	CompressedBitConnector::Load(char *fileName)
{
	CompressedBitConnectorHeader	h;
	while (true)
	{
		if (CacheFileName.Set(fileName)==false)
			break;	
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
	
		if (CacheMemory!=0)
		{
			notifier->Info("[%s] -> Using %d bytes for cache",ObjectName,CacheMemory);
			if (AllocMemory(CacheMemory)==false)
			{
				notifier->Error("[%s] -> Unable to allocate space for cache memory (%d)",ObjectName,CacheMemory);
				break;
			}
			if (file.SetFilePos((UInt64)sizeof(h)+(UInt64)MemToAlloc)==false)
				break;			
		} else {
			if (AllocMemory(h.MemToAlloc)==false)
			{
				notifier->Error("[%s] -> Unable to allocate space for cache initialization",ObjectName);
				break;
			}
			if (file.Read(Data,(UInt64)MemToAlloc)==false)
				break;
		}	
		if (file.Read(Indexes,sizeof(UInt64)*(UInt64)nrRecords)==false)
			break;
		if (file.Read(Labels.GetData(),Labels.GetAllocated())==false)
			break;
		if (LoadRecordHashesAndFeatureNames(&h)==false)
			break;
		CloseCacheFile();
		if (CacheMemory!=0)
		{
			dataMemorySize = (UInt64)nrRecords * sizeof(UInt64) + CacheMemory+Labels.GetAllocated();
		} else {
			dataMemorySize = (UInt64)nrRecords * sizeof(UInt64) + MemToAlloc+Labels.GetAllocated();
		}		
		
		return true;		
	}
	ClearColumnIndexes();
	CloseCacheFile();
	notifier->Error("[%s] -> Error read data from %s",ObjectName,fileName);
	return false;
}
bool	CompressedBitConnector::UpdateCacheMemory(CompressedBitConnectorThreadCacheData &ibthData,UInt64 start,UInt64 szBuffer)
{
	UInt64	sz = 0;
	
	while (true)
	{		
		if (start>MemToAlloc)
			break;
		sz = CacheMemory;
		if (start+sz>MemToAlloc)
			sz = MemToAlloc-start;
		if (sz<szBuffer)
			break;
		//notifier->Info("[%s] -> Updateing cache : [%d-%d]",ObjectName,(int)start,(int)(start+sz));
		if (ibthData.CacheFile.Read(start+sizeof(CompressedBitConnectorHeader),ibthData.Data,sz)==false)
			break;
		ibthData.CacheStart = start;
		ibthData.CacheEnd = start+sz;
		return true;
	}
	ibthData.CacheStart = ibthData.CacheEnd = INVALID_CACHE_INDEX;
	notifier->Error("[%s] -> Unable to update cache ...",ObjectName);
	return false;
}
bool	CompressedBitConnector::CreateMlRecord (GML::ML::MLRecord &record)
{
	record.ThreadData = NULL;
	record.FeatCount = columns.nrFeatures;	
	if ((record.Features = new double[columns.nrFeatures])==NULL)
	{
		notifier->Error("[%s] -> Unable to create mlRecord: error allocing %d values for features !",ObjectName,columns.nrFeatures);
		return false;
	}
	if (CacheMemory>0)
	{
		if ((record.ThreadData = new UInt8[sizeof(CompressedBitConnectorThreadCacheData)+CacheMemory])==NULL)
		{
			notifier->Error("[%s] -> Unable to create mlRecord: error allocing %d bytes for cache",ObjectName,(UInt32)CacheMemory);
			return false;
		}
		CompressedBitConnectorThreadCacheData *ibthData = (CompressedBitConnectorThreadCacheData *)record.ThreadData;
		ibthData->CacheStart = INVALID_CACHE_INDEX;
		ibthData->CacheEnd = INVALID_CACHE_INDEX;
		if (ibthData->CacheFile.OpenRead(CacheFileName.GetText(),true)==false)
		{
			DWORD err = GetLastError();
			notifier->Error("[%s] -> Unable to create mlRecord: error opening %s for caching - %d",ObjectName,CacheFileName.GetText(),err);
			return false;
		}
	}
	return true;	
}
bool	CompressedBitConnector::GetRecord(GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask)
{
	UInt8									*cPoz,*end;
	UInt32									indexFeat,Last;
	UInt64									sz,iPoz;	
	CompressedBitConnectorThreadCacheData	*ibthData;

	if (index>=nrRecords)
		return false;
	
	if (index+1==nrRecords)
	{
		sz = MemToAlloc -Indexes[index];
	} else {
		sz = Indexes[index+1]-Indexes[index];
	}
	if (CacheMemory==0)
	{
		cPoz = &Data[Indexes[index]];
	} else {
		// verific daca e in cache, updatez
		iPoz = Indexes[index];
		ibthData = (CompressedBitConnectorThreadCacheData *)record.ThreadData;
		if ((iPoz<ibthData->CacheStart) || (iPoz+sz>=ibthData->CacheEnd))
		{
			if (UpdateCacheMemory(*ibthData,iPoz,sz)==false)
			{
				notifier->Error("[%s] -> Unable to update cache memory for location: %d, ",ObjectName,(int)iPoz);
				return false;			
			}
		}
		cPoz = &ibthData->Data[iPoz-ibthData->CacheStart];
	}	
	end = cPoz+sz;
	memset(record.Features,0,sizeof(double)*columns.nrFeatures);
	Last = 0;
	while (cPoz<end)
	{
		switch (Method)
		{
			case METHOD_INT8_INDEX:
				indexFeat = Last+(UInt32)(*cPoz);
				cPoz++;
				break;
			case METHOD_INT16_INDEX:
				indexFeat = Last+(*((UInt16*)cPoz));
				cPoz+=sizeof(UInt16);
				break;
			case METHOD_INT32_INDEX:
				indexFeat = Last+(*((UInt32*)cPoz));
				cPoz+=sizeof(UInt32);
				break;
			case METHOD_254_BASE_INDEX:
				COMPUTE_INDEX_FEAT(254);
				break;
			case METHOD_253_BASE_INDEX:
				COMPUTE_INDEX_FEAT(253);
				break;
			case METHOD_252_BASE_INDEX:
				COMPUTE_INDEX_FEAT(252);
				break;
			case METHOD_251_BASE_INDEX:
				COMPUTE_INDEX_FEAT(251);
				break;
			case METHOD_250_BASE_INDEX:
				COMPUTE_INDEX_FEAT(250);
				break;		
			case METHOD_255_BASE_INDEX:
				if ((*cPoz)<255)
				{
					indexFeat = Last+(*cPoz);
					cPoz++;
				} else {
					cPoz++;
					indexFeat = Last+(*((UInt16*)cPoz));
					cPoz+=sizeof(UInt16);
				}
				break;	
			case METHOD_INT15_EXTEND_INDEX:
				if ((*cPoz)<128)
				{
					indexFeat = (*cPoz);
					cPoz++;
				} else {
					indexFeat = (((UInt32)cPoz[1])<<7)|((*cPoz) & 127);
					cPoz+=2;
				}
				indexFeat+=Last;
				break;					
			default:
				notifier->Error("[%s] -> Unknwon compressed method : %d ",ObjectName,Method);
				return false;
		};
		if (indexFeat>=columns.nrFeatures)
		{
			notifier->Error("[%s] -> Invalid index for feature : %d ",ObjectName,indexFeat);
			return false;
		}
		record.Features[indexFeat] = 1.0;
		Last = indexFeat;
	}


	// pun si label-ul
	if (Labels.Get(index))
		record.Label = OutLabelPositive;
	else
		record.Label = OutLabelNegative;

	return UpdateRecordExtraData(record,index,recordMask);
}
bool	CompressedBitConnector::GetRecordLabel(double &Label,UInt32 index)
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
bool	CompressedBitConnector::FreeMLRecord(GML::ML::MLRecord &record)
{
	if (record.Features!=NULL)
	{
		delete record.Features;
		record.Features = NULL;
	}
	if ((CacheMemory>0) && (record.ThreadData!=NULL))
	{
		((CompressedBitConnectorThreadCacheData *)record.ThreadData)->CacheFile.Close();
		delete record.ThreadData;
		record.ThreadData = NULL;
		
	}
	return true;
}
