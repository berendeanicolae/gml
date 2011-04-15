#include "LinearVote.h"


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

	SetPropertyMetaData("Command","!!LIST:None=0,CreateCache!!");
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
	return true;
}
bool LinearVote::Init()
{
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
	return true;
}
void LinearVote::OnExecute()
{
	if (Command==1)	//CreateCache
	{
		if (con!=NULL)
		{

		}
	}
}