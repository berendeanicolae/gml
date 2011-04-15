#include "LinearVote.h"

void ThreadRedirectFunction(GML::Utils::IParalelUnit *paralel,void *context)
{
	LinearVote *lv = (LinearVote *)context;
	lv->OnRunThreadCommand(lv->ptData[paralel->GetID()],paralel->GetCodeID());
}

//================================================================================
PerceptronVector::PerceptronVector()
{
	Weight = NULL;
	Bias = 0;
	Count = 0;
	Vote = 0;
}
PerceptronVector::PerceptronVector(PerceptronVector &ref)
{
	Weight = NULL;
	Bias = 0;
	Count = 0;
	Vote = 0;

	if (Create(ref.Count))
	{
		Bias = ref.Bias;
		memcpy(Weight,ref.Weight,sizeof(double)*ref.Count);
		Count = ref.Count;
		Vote = ref.Vote;
		FileName.Set(&ref.FileName);
	}	
}
bool PerceptronVector::operator > (PerceptronVector &r)
{
	return (bool)(Vote>r.Vote);
}
bool PerceptronVector::operator < (PerceptronVector &r)
{
	return (bool)(Vote<r.Vote);
}
PerceptronVector::~PerceptronVector()
{
	Destroy();
}
void PerceptronVector::Destroy()
{
	if (Weight!=NULL)
		delete Weight;
	Bias = 0;
	Weight = NULL;
	Bias = NULL;
	Count = 0;
	Vote = 0;
}
bool PerceptronVector::Create(UInt32 count)
{
	Destroy();
	if ((Weight = new double[count])==NULL)
		return false;
	Bias = 0.0;
	memset(Weight,0,sizeof(double)*count);
	Vote = 0;
	Count = count;

	return true;
}
//================================================================================

LinearVote::LinearVote()
{
	ObjectName = "LinearVote";

	SetPropertyMetaData("Command","!!LIST:Test=0!!");
	LinkPropertyToString("DataBase"					,DataBase				,"");
	LinkPropertyToString("Connector"				,Conector				,"");
	LinkPropertyToString("Notifier"					,Notifier				,"");
	LinkPropertyToString("WeightFileList"			,WeightFiles			,"","A list of weight files to be loaded separated by a comma.");
	LinkPropertyToString("VotePropertyName"			,VotePropertyName		,"Vote","The name of the property that contains the vote. It has to be a numeric property.");
	LinkPropertyToUInt32("ThreadsCount"				,threadsCount			,1,"");
}
bool LinearVote::Create(PerceptronVector &pv,char *fileName)
{
	GML::Utils::AttributeList	attr;
	GML::Utils::Attribute		*a;

	if (pv.Create(con->GetFeatureCount())==false)
	{
		notif->Error("[%s] -> Unable to create PerceptronVector object ",ObjectName);
		return false;
	}
	if (attr.Load(fileName)==false)
	{
		notif->Error("[%s] -> Unable to load / Invalid format for %s ",ObjectName,fileName);
		return false;
	}
	if (attr.UpdateDouble("Bias",pv.Bias)==false)
	{
		notif->Error("[%s] -> Missing 'Bias' field in %s ",ObjectName,fileName);
		return false;
	}
	if (attr.Update("Weight",pv.Weight,sizeof(double)*con->GetFeatureCount())==false)
	{
		notif->Error("[%s] -> Missing 'Weight' field or different count of Weights in %s ",ObjectName,fileName);
		return false;
	}
	if (pv.FileName.Set(fileName)==false)
	{
		notif->Error("[%s] -> Unable to set FileName property for PeceptronVector",ObjectName);
		return false;
	}
	// vote
	if ((a = attr.Get(VotePropertyName.GetText()))==NULL)
	{
		notif->Error("[%s] -> Missing '%s' property in %s",ObjectName,VotePropertyName.GetText(),fileName);
		return false;
	}
	switch (a->AttributeType)
	{
		case GML::Utils::AttributeList::DOUBLE:
			pv.Vote = (double)(*(double *)a->Data);
			break;
		case GML::Utils::AttributeList::FLOAT:
			pv.Vote = (double)(*(float *)a->Data);
			break;
		case GML::Utils::AttributeList::UINT8:
			pv.Vote = (double)(*(UInt8 *)a->Data);
			break;
		case GML::Utils::AttributeList::INT8:
			pv.Vote = (double)(*(Int8 *)a->Data);
			break;
		case GML::Utils::AttributeList::UINT16:
			pv.Vote = (double)(*(UInt16 *)a->Data);
			break;
		case GML::Utils::AttributeList::INT16:
			pv.Vote = (double)(*(Int16 *)a->Data);
			break;
		case GML::Utils::AttributeList::UINT32:
			pv.Vote = (double)(*(UInt32 *)a->Data);
			break;
		case GML::Utils::AttributeList::INT32:
			pv.Vote = (double)(*(Int32 *)a->Data);
			break;
		case GML::Utils::AttributeList::UINT64:
			pv.Vote = (double)(*(UInt64 *)a->Data);
			break;
		case GML::Utils::AttributeList::INT64:
			pv.Vote = (double)(*(Int64 *)a->Data);
			break;
		default:
			notif->Error("[%s] -> Property '%s' should be a numeric type (in %s)",ObjectName,VotePropertyName.GetText(),fileName);
			return false;
	}
	return true;
}
bool LinearVote::LoadVotes()
{
	GML::Utils::GString		tmp;
	int						poz,count;
	PerceptronVector		pv;

	pVectors.DeleteAll();
	if (WeightFiles.Len()==0)
	{
		notif->Error("[%s] -> You need to set up 'WeightFiles' property with a list of files !",ObjectName);
		return false;
	}

	poz = 0;
	count = 0;
	while (WeightFiles.CopyNext(&tmp,";",&poz))
	{
		tmp.Strip();
		if (tmp.Len()>0)
			count++;
	}
	if (count==0)
	{
		notif->Error("[%s] -> You need to set up 'WeightFiles' property with a list of files !",ObjectName);
		return false;
	}

	poz = 0;
	while (WeightFiles.CopyNext(&tmp,";",&poz))
	{
		tmp.Strip();
		if (tmp.Len()>0)
		{
			if (Create(pv,tmp.GetText())==false)
				return false;	
			if (pVectors.PushByRef(pv)==false)
			{
				notif->Error("[%s] -> Unable to add Vector to list !",ObjectName);
				return false;
			}
			
		}
	}
	notif->Info("[%s] -> Total vectors: %d",ObjectName,pVectors.Len());

	// creez si indexii
	if (indexes.Create(pVectors.Len())==false)
	{
		notif->Error("[%s] -> Unable to create indexes lists !",ObjectName);
		return false;
	}
	for (UInt32 tr=0;tr<pVectors.Len();tr++)
		if (indexes.Push(tr)==false)
		{
			notif->Error("[%s] -> Unable to add index #%d to list !",ObjectName,tr);
			return false;
		}
	return true;
}
void LinearVote::OnRunThreadCommand(ThreadData &td,UInt32 command)
{
	switch (command)
	{
		case PARALLEL_CMD_TEST:
			PerformTest(td);
			break;
	}
}
bool LinearVote::Init()
{
	UInt32	tr;
	UInt32	splitValue,start;

	if ((notif = GML::Builder::CreateNotifier(Notifier.GetText()))==NULL)
		return false;
	if (DataBase.Len()!=0)
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
	if (LoadVotes()==false)
		return false;
	if (threadsCount<1)
		threadsCount = 1;

	if ((ptData = new ThreadData[threadsCount])==NULL)
	{
		notif->Error("[%s] -> Unable to allocate ThreadData[%d]",ObjectName,threadsCount);
		return false;
	}

	splitValue = (con->GetRecordCount()/threadsCount);
	start = 0;
	for (tr=0;tr<threadsCount;tr++)
	{
		if (con->CreateMlRecord(ptData[tr].Record)==false)
		{
			notif->Error("[%s] -> Unable to create ThreadData[%d].Record",ObjectName,tr);
			return false;
		}
		if ((start+splitValue)>con->GetRecordCount())
			splitValue = con->GetRecordCount()-start;
		ptData[tr].Range.Set(start,start+splitValue);
		start+=splitValue;
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
	return true;
}
bool LinearVote::ExecuteParalelCommand(UInt32 command)
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
bool LinearVote::PerformTest(ThreadData &td)
{
	UInt32				index,tr,nrFeatures,nrVectors;
	PerceptronVector	*pv;
	double				scorPozitive,scorNegative,result;
	double				*scor;
	

	index = td.Range.Start;
	td.Res.Clear();
	nrFeatures = con->GetFeatureCount();
	nrVectors = indexes.Len();

	while (index<td.Range.End)
	{
		if (con->GetRecord(td.Record,index)==false)
		{
			notif->Error("[%s] -> Unable to read record %d",ObjectName,tr);
			return false;
		}
		for (tr=0;tr<nrVectors;tr++)
		{
			pv = pVectors.GetPtrToObject(tr);
			if ((GML::ML::VectorOp::ComputeVectorsSum(td.Record.Features,pv->Weight,nrFeatures)+pv->Bias)>0)
				scor = &scorPozitive;				
			else
				scor = &scorNegative;
			(*scor)+=pv->Vote;
		}
		if (scorPozitive>scorNegative)
			result = 1;
		else
			result = -1;
		td.Res.Update(td.Record.Label==1,(bool)((result * td.Record.Label)>0));	
		index++;
	}
}
void LinearVote::DoTest()
{
	GML::Utils::AlgorithmResult		res;
	UInt32							tr;

	res.Clear();
	ExecuteParalelCommand(PARALLEL_CMD_TEST);
	for (tr=0;tr<threadsCount;tr++)
		res.Add(&ptData[tr].Res);
	res.Compute();
	notif->Notify(100,&res,sizeof(res));
}
void LinearVote::OnExecute()
{
	if (Command==0)	//CreateCache
	{
		DoTest();
	}
}