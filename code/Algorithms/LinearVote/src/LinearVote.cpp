#include "LinearVote.h"

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

	SetPropertyMetaData("Command","!!LIST:None=0,Test!!");
	LinkPropertyToString("WeightFileList"			,WeightFiles			,"","A list of weight files to be loaded separated by a comma.");
	LinkPropertyToString("VotePropertyName"			,VotePropertyName		,"Vote","The name of the property that contains the vote. It has to be a numeric property.");
	LinkPropertyToUInt32("VotesLoadingMethod"		,VotesLoadingMethod		,0,"!!LIST:FromList=0,FromPath!!");
	LinkPropertyToUInt32("VoteComputeMethod"		,VoteComputeMethod		,VOTE_COMPUTE_ADDITION,"!!LIST:Add=0,Multiply,Count!!");
	LinkPropertyToUInt32("OnEqualVotes"				,VoteOnEqual			,VOTE_NEGATIVE,"!!LIST:VoteNegative=0,VotePositive!!\nSets the vote that will be considered in case of equal votes");
	LinkPropertyToString("WeightPath"				,WeightPath				,"*.txt","The path where the weigh files are");
	LinkPropertyToDouble("PositiveVoteFactor"		,PositiveVoteFactor		,1.0,"Factor used to multiply the positive votes");
	LinkPropertyToDouble("NegativeVoteFactor"		,NegativeVoteFactor		,1.0,"Factor used to multiply the negative votes");


	// Hash-uri
	LinkPropertyToUInt32("HashSelectMethod"			,HashSelectMethod		,HASH_SELECT_NONE,"!!LIST:None=0,All,CorectelyClasify,IncorectelyClasify,Positive,Negative,PositiveCorectelyClasify,PositiveInCorectelyClasify,NegativeCorectelyClasify,NegativeInCorectelyClasify!!");
	AddHashSavePropery();
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
bool LinearVote::LoadVotesFromWeightPath()
{
	HANDLE					hFile;
	WIN32_FIND_DATA			dt;
	GML::Utils::GString		tmp;
	PerceptronVector		pv;

	pVectors.DeleteAll();
	if ((hFile=FindFirstFileA(WeightPath.GetText(),&dt))==INVALID_HANDLE_VALUE)
	{
		notif->Error("[%s] -> Invalid path : %s !",ObjectName,WeightPath.GetText());
		return false;
	}
	do
	{
		// daca e fisier , il incarc
		if ((dt.dwFileAttributes & 16)==0)
		{
			WeightPath.CopyPathName(&tmp);
			tmp.PathJoinName(dt.cFileName);
			notif->Info("[%s] -> Loading : %s",ObjectName,tmp.GetText());
			if (Create(pv,tmp.GetText())==false)
				return false;	
			if (pVectors.PushByRef(pv)==false)
			{
				notif->Error("[%s] -> Unable to add Vector to list !",ObjectName);
				return false;
			}
		}
	} while (FindNextFile(hFile,&dt));
	FindClose(hFile);
	return true;
}
bool LinearVote::LoadVotesFromList()
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
		if (tmp.GetText()[0]!=0)
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
		if (tmp.GetText()[0]!=0)
		{
			notif->Info("[%s] -> Loading : %s",ObjectName,tmp.GetText());
			if (Create(pv,tmp.GetText())==false)
				return false;	
			if (pVectors.PushByRef(pv)==false)
			{
				notif->Error("[%s] -> Unable to add Vector to list !",ObjectName);
				return false;
			}			
		}
	}

	return true;
}
void LinearVote::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 command)
{
	switch (command)
	{
		case PARALLEL_CMD_TEST:
			PerformTest(thData);
			break;
	}
}
bool LinearVote::CheckValidVotes()
{
	for (UInt32 tr=0;tr<indexes.Len();tr++)
	{
		PerceptronVector *pv = pVectors.GetPtrToObject(indexes.Get(tr));
		if (pv==NULL)
			continue;
		if ((pv->Vote==0) && (VoteComputeMethod == VOTE_COMPUTE_MULTIPLY))
		{
			notif->Error("[%s] -> Invalid Vote value (0.0) for Multiply method on %s",ObjectName,pv->FileName.GetText());
			return false;
		}
	}
	return true;
}
bool LinearVote::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	if ((thData.Context = new LinearVoteThreadData())==NULL)
		return false;

	((LinearVoteThreadData *)thData.Context)->eqVotes = 0;
	return true;
}
bool LinearVote::Init()
{
	if (InitConnections()==false)
		return false;
	if (InitThreads()==false)
		return false;
	if (SplitMLThreadDataRange(con->GetRecordCount())==false)
		return false;
	if (PositiveVoteFactor<=0)
	{
		notif->Error("[%s] -> 'PositiveVoteFactor' shoulb be bigger than 0.0");
		return false;
	}
	if (NegativeVoteFactor<=0)
	{
		notif->Error("[%s] -> 'NegativeVoteFactor' shoulb be bigger than 0.0");
		return false;
	}

	if (VotesLoadingMethod==LOAD_VOTES_FROMLIST)
	{
		if (LoadVotesFromList()==false)
			return false;
	} else {
		if (LoadVotesFromWeightPath()==false)
			return false;
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
	if (CheckValidVotes()==false)
		return false;
	if (HashSelectMethod!=HASH_SELECT_NONE)
	{
		if (RecordsStatus.Create(con->GetRecordCount())==false)
		{
			notif->Error("[%s] -> Unable to create Status Record for %d records ",ObjectName,con->GetRecordCount());
			return false;
		}
		if (RecordsStatus.Resize(con->GetRecordCount())==false)
		{
			notif->Error("[%s] -> Unable to alloc Status Record for %d records ",ObjectName,con->GetRecordCount());
			return false;
		}
		memset(RecordsStatus.GetVector(),0,RecordsStatus.Len());
	}


	return true;
}
bool LinearVote::PerformTest(GML::Algorithm::MLThreadData &td)
{
	UInt32					index,tr,nrFeatures,nrVectors;
	PerceptronVector		*pv;
	LinearVoteThreadData	*ltd;
	double					scorPozitive,scorNegative,result;
	double					*scor;
	bool					corectelyClasified;
	UInt8					*rStatus = RecordsStatus.GetVector();
	
	ltd = (LinearVoteThreadData *)td.Context;
	index = td.Range.Start;
	td.Res.Clear();
	ltd->eqVotes = 0;
	nrFeatures = con->GetFeatureCount();
	nrVectors = indexes.Len();
	scorPozitive = scorNegative = 0;

	while (index<td.Range.End)
	{
		if (con->GetRecord(td.Record,index)==false)
		{
			notif->Error("[%s] -> Unable to read record %d",ObjectName,index);
			return false;
		}
		switch (VoteComputeMethod)
		{
			case VOTE_COMPUTE_ADDITION:
			case VOTE_COMPUTE_COUNT:
				scorPozitive = scorNegative = 0;
				break;
			case VOTE_COMPUTE_MULTIPLY:
				scorPozitive = scorNegative = 1;
				break;
		};		
		for (tr=0;tr<nrVectors;tr++)
		{
			pv = pVectors.GetPtrToObject(tr);
			if ((GML::ML::VectorOp::ComputeVectorsSum(td.Record.Features,pv->Weight,nrFeatures)+pv->Bias)>0)
				scor = &scorPozitive;				
			else
				scor = &scorNegative;
			
			// ajustez votul
			switch (VoteComputeMethod)
			{
				case VOTE_COMPUTE_ADDITION:
					(*scor)+=pv->Vote;
					break;
				case VOTE_COMPUTE_COUNT:
					(*scor)+=1;
					break;
				case VOTE_COMPUTE_MULTIPLY:
					(*scor)*=pv->Vote;
					break;
			};
		}
		scorPozitive *= PositiveVoteFactor;
		scorNegative *= NegativeVoteFactor;
		if (scorPozitive==scorNegative)
		{
			ltd->eqVotes++;
			if (VoteOnEqual==VOTE_POZITIVE)
				scorNegative++;
			else
				scorPozitive++;
		}
		if (scorPozitive>scorNegative)
			result = 1;
		else 
			result = -1;
		corectelyClasified = (bool)((result * td.Record.Label)>0);
		td.Res.Update(td.Record.Label==1,corectelyClasified);	
		switch (HashSelectMethod)
		{
			case HASH_SELECT_NONE:
				break;
			case HASH_SELECT_ALL:
				rStatus[index] = 1;
				break;
			case HASH_SELECT_CORECTELY_CLASIFY:
				if (corectelyClasified)
					rStatus[index] = 1;
				break;
			case HASH_SELECT_INCORECTELY_CLASIFY:
				if (corectelyClasified==false)
					rStatus[index] = 1;
				break;
			case HASH_SELECT_POSITIVE:
				if (td.Record.Label==1)
					rStatus[index] = 1;
				break;
			case HASH_SELECT_NEGATIVE:
				if (td.Record.Label!=1)
					rStatus[index] = 1;
				break;
			case HASH_SELECT_POSITIVE_CORECTELY_CLASIFY:
				if ((td.Record.Label==1) && (corectelyClasified))
					rStatus[index] = 1;
				break;
			case HASH_SELECT_POSITIVE_INCORECTELY_CLASIFY:
				if ((td.Record.Label==1) && (corectelyClasified==false))
					rStatus[index] = 1;
				break;
			case HASH_SELECT_NEGATIVE_CORECTELY_CLASIFY:
				if ((td.Record.Label!=1) && (corectelyClasified))
					rStatus[index] = 1;
				break;
			case HASH_SELECT_NEGATIVE_INCORECTELY_CLASIFY:
				if ((td.Record.Label!=1) && (corectelyClasified==false))
					rStatus[index] = 1;
				break;
		};
		index++;
	}

	return true;
}
void LinearVote::DoTest()
{
	GML::Utils::AlgorithmResult		res;
	UInt32							tr,eqVotes;

	res.Clear();
	res.time.Start();
	ExecuteParalelCommand(PARALLEL_CMD_TEST);
	for (tr=0,eqVotes=0;tr<threadsCount;tr++)
	{
		res.Add(&ThData[tr].Res);
		eqVotes+=((LinearVoteThreadData *)ThData[tr].Context)->eqVotes;
	}
	res.Compute();
	res.time.Stop();
	notif->Info("[%s] -> Equal votes : %d",ObjectName,eqVotes);
	notif->Result(res);
	if (HashSelectMethod!=HASH_SELECT_NONE)
		SaveHashResult(HashFileName.GetText(),HashFileType,RecordsStatus);
}
void LinearVote::OnExecute()
{
	if (Command==1)	//CreateCache
	{
		DoTest();
		return;
	}
	notif->Error("[%s] -> Unknown command ID: %d",ObjectName,Command);
}