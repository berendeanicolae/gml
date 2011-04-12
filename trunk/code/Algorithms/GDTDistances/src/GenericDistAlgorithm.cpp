#include "GenericDistAlgorithm.h"

void ThreadRedirectFunction(GML::Utils::IParalelUnit *paralel,void *context)
{
	GenericDistAlgorithm *gp = (GenericDistAlgorithm *)context;
	gp->OnRunThreadCommand(gp->ptData[paralel->GetID()],paralel->GetCodeID());
}
//==============================================================================
GenericDistAlgorithm::GenericDistAlgorithm()
{
	ObjectName = "GenericDistance";

	db = NULL;
	con = NULL;

	SetPropertyMetaData("Command","!!LIST:None=0,Compute!!");

	LinkPropertyToString("DataBase"					,DataBase				,"");
	LinkPropertyToString("Connector"				,Conector				,"");
	LinkPropertyToString("Notifier"					,Notifier				,"");
	LinkPropertyToString("HashFileName"				,HashFileName			,"Name of the file with the record hash list result.");
	LinkPropertyToUInt32("HashStoreMethod"			,HashStoreMethod		,0,"!!LIST:SaveAsTextFile=0,SaveAsBinaryFile!!");

}
bool	GenericDistAlgorithm::CreatePozitiveAndNegativeIndexes()
{
	UInt32	tr;
	UInt32	pozitiveCount,negativeCount,nrRecords;
	double	label;
	bool	res;

	notif->Info("[%s] -> Creating Pozitive and Negative indexes ... ",ObjectName);
	pozitiveCount = negativeCount = 0;
	nrRecords = con->GetRecordCount();
	
	for (tr=0;tr<nrRecords;tr++)
	{
		if (con->GetRecordLabel(label,tr)==false)
		{
			notif->Error("[%s] -> Unable to read label for record #%d",ObjectName,tr);
			return false;
		}
		if (label==1.0)
			pozitiveCount++;
		else
			negativeCount++;
	}
	notif->Info("[%s] -> Pozitive = %d ,Negative = %d",ObjectName,pozitiveCount,negativeCount);
	if (indexesPozitive.Create(pozitiveCount)==false)
	{
		notif->Error("[%s] -> Unable to alloc %d pozitive indexes",ObjectName,pozitiveCount);
		return false;
	}
	if (indexesNegative.Create(negativeCount)==false)
	{
		notif->Error("[%s] -> Unable to alloc %d negative indexes",ObjectName,negativeCount);
		return false;
	}
	// adaugam si indexii
	for (tr=0;tr<nrRecords;tr++)
	{
		if (con->GetRecordLabel(label,tr)==false)
		{
			notif->Error("[%s] -> Unable to read label for record #%d",ObjectName,tr);
			return false;
		}
		if (label==1.0)
			res = indexesPozitive.Push(tr);
		else
			res = indexesNegative.Push(tr);
		if (res==false)
		{
			notif->Error("[%s] -> Unable to add index #%d",ObjectName,tr);
			return false;
		}		
	}
	notif->Info("[%s] -> Pozitive and Negative indexes created !",ObjectName);
	return true;
}
bool	GenericDistAlgorithm::Init()
{
	UInt32		tr;

	// creez obiectele:
	if ((notif = GML::Builder::CreateNotifier(Notifier.GetText()))==NULL)
		return false;
	if (DataBase.Len()>0)
	{
		if ((db = GML::Builder::CreateDataBase(DataBase.GetText(),*notif))==NULL)
		{
			notif->Error("[%s] -> Unable to create Database (%s)",ObjectName,DataBase.GetText());
			return false;
		}
		if (db->Connect()==false)
		{
			notif->Error("[%s] -> Unable to connesct to Database (%s)",ObjectName,DataBase.GetText());
			return false;
		}
		if ((con = GML::Builder::CreateConnectors(Conector.GetText(),*notif,*db))==NULL)
		{
			notif->Error("[%s] -> Unable to create Conector (%s)",ObjectName,Conector.GetText());
			return false;
		}
	} else {
		if ((con = GML::Builder::CreateConnectors(Conector.GetText(),*notif))==NULL)
		{
			notif->Error("[%s] -> Unable to create Conector (%s)",ObjectName,Conector.GetText());
			return false;
		}
	}
	if (threadsCount<1)
		threadsCount = 1;
	
	if (RecordsStatus.Create(con->GetRecordCount())==false)
	{
		notif->Error("[%s] -> Unable to create RecordsStatus vector (for %d records)",ObjectName,con->GetRecordCount());
		return false;
	}
	// resetare
	if (RecordsStatus.Resize(con->GetRecordCount())==false)
	{
		notif->Error("[%s] -> Unable to alloc RecordsStatus vector (for %d records)",ObjectName,con->GetRecordCount());
		return false;
	}
	// setare totul pe 0
	memset(RecordsStatus.GetVector(),0,sizeof(UInt8) * RecordsStatus.Len());

	if ((ptData = new ThreadData[threadsCount])==NULL)
	{
		notif->Error("[%s] -> Unable to allocate ThreadData[%d]",ObjectName,threadsCount);
		return false;
	}
	for (tr=0;tr<threadsCount;tr++)
	{
		ptData[tr].ID = tr;
		if (con->CreateMlRecord(ptData[tr].Rec1)==false)
		{
			notif->Error("[%s] -> Unable to create ThreadData[%d].Rec1",ObjectName,tr);
			return false;
		}
		if (con->CreateMlRecord(ptData[tr].Rec2)==false)
		{
			notif->Error("[%s] -> Unable to create ThreadData[%d].Rec2",ObjectName,tr);
			return false;
		}
	}
		
	if ((tpu = new GML::Utils::ThreadParalelUnit[threadsCount])==NULL)
	{
		notif->Error("[%s] -> Unable to create %d threads ",ObjectName,threadsCount);
		return false;
	}
	for (tr=0;tr<threadsCount;tr++)
	{
		if (tpu[tr].Init(tr,this,ThreadRedirectFunction)==false)
		{
			notif->Error("[%s] -> Unable to start thread #%d",ObjectName,tr);
			return false;
		}
	}

	return OnInit();
}
bool    GenericDistAlgorithm::ExecuteParalelCommand(UInt32 command)
{
	UInt32	tr;

	// executie
	for (tr=0;tr<threadsCount;tr++)
		if (tpu[tr].Execute(command)==false)
		{
			notif->Error("[%s] -> Error on runnig thread #%d",ObjectName,tr);
			return false;
		}
	// asteptare
	for (tr=0;tr<threadsCount;tr++)
		if (tpu[tr].WaitToFinish()==false)
		{
			notif->Error("[%s] -> WaitToFinish failed on thread #%d",ObjectName,tr);
			return false;
		}
	// all ok
	return true;
}
bool	GenericDistAlgorithm::OnInit()
{
	return true;
}
void	GenericDistAlgorithm::OnExecute()
{
	StopAlgorithm = false;
	
	switch (Command)
	{
		case COMMAND_NONE:
			notif->Info("[%s] -> Nothing to do ... ",ObjectName);
			return;
		case COMMAND_COMPUTE:
			OnCompute();
			return;
	};
	notif->Error("[%s] -> Unkwnown command ID : %d",ObjectName,Command);
}
bool	GenericDistAlgorithm::SaveHashResult()
{
	GML::Utils::File		f;
	GML::Utils::Vector		Cache;
	UInt32					recSize,tr,count;
	UInt8					*status,*p;
	GML::DB::RecordHash		rHash;
	GML::Utils::GString		temp;

	switch (HashStoreMethod)
	{
		case SAVE_HASHES_AS_TEXT:
			recSize = 32+2;
			break;
		case SAVE_HASHES_AS_BINARY:
			recSize = 16;
			break;
		default:
			notif->Error("[%s] -> Unknown HashStoreMethod = %d",ObjectName,HashStoreMethod);
			return false;
	};
	status = RecordsStatus.GetVector();
	for (tr=0,count=0;tr<RecordsStatus.Len();tr++,status++)
	{
		if ((*status)!=0)
			count++;
	}
	if (HashFileName.Len()==0)
	{
		notif->Error("[%s] -> You need to set 'HashFileName' property with the name of the file where you want to save your results",ObjectName);
		return false;
	}
	if (f.Create(HashFileName.GetText())==false)
	{
		notif->Error("[%s] -> Unable to create %s",ObjectName,HashFileName.GetText());
		return false;
	}
	status = RecordsStatus.GetVector();
	while (true)
	{
		if (Cache.Create(SAVE_CACHE_SIZE,recSize)==false)
		{
			notif->Error("[%s] -> Unable to allocate %d bytes for caching ...",ObjectName,recSize * SAVE_CACHE_SIZE);
			break;
		}
		// pentru binary method , scriu cateva date
		if (HashStoreMethod==SAVE_HASHES_AS_BINARY)
		{
			if (f.Write("HASHLIST",8)==false)
				break;
			if (f.Write(&count,sizeof(UInt32))==false)
				break;
		}
		for (tr=0;tr<RecordsStatus.Len();tr++,status++)
		{
			if ((*status)!=0)
			{
				// scriu datele
				if (con->GetRecordHash(rHash,tr)==false)
				{
					notif->Error("[%s] -> Unable to read hash for record #%d",ObjectName,tr);
					break;
				}
				switch (HashStoreMethod)
				{
					case SAVE_HASHES_AS_TEXT:
						rHash.ToString(temp);
						p = (UInt8 *)temp.GetText();
						break;
					case SAVE_HASHES_AS_BINARY:
						recSize = 16;
						p = (UInt8 *)&rHash.Hash.bValue[0];
						break;
				};
				if (Cache.GetSize()>=SAVE_CACHE_SIZE)
				{
					if (f.Write(Cache.GetVector(),Cache.GetSize()*recSize)==false)
						break;
					if (Cache.Resize(0)==false)
						break;
				}
				if (Cache.Push(p)==false)
					break;
			}
		}
		// daca nu am ajuns la final , am o eroare
		if (tr!=RecordsStatus.Len())
			break;
		if (Cache.GetSize()>=0)
		{
			if (f.Write(Cache.GetVector(),Cache.GetSize()*recSize)==false)
				break;
		}
		f.Close();
		notif->Info("[%s] Hashes saved ok in %s (%d hashesh)",ObjectName,HashFileName.GetText(),count);
		return true;
	}
	f.Close();
	DeleteFileA(HashFileName.GetText());
	notif->Error("[%s] -> Unable to write hashes to %s",ObjectName,HashFileName.GetText());
	return false;
}