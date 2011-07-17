#include "Container.h"

/*
Formatul este:
	NumeFeature,type,cID,fID,expresion
*/

struct Header
{
	UInt32	Magic;
	UInt32	nrRecords;
	UInt32	Rezerved;
};
struct StructFeatTypes
{
	char*	Name;
	UInt32	Value;
};
static StructFeatTypes FeatTypes[] = 
{
	{"bool",	GML::DB::TYPES::BOOLEAN},
	{"uint32",	GML::DB::TYPES::UINT32},
	{"dword",	GML::DB::TYPES::UINT32},
	{"uint",	GML::DB::TYPES::UINT32},
	{"int32",	GML::DB::TYPES::INT32},
	{"int",		GML::DB::TYPES::INT32},
};
struct OP_INFO
{
	char	*Name;
	UInt32	ID;
	UInt32	MinParams;
	UInt32	MaxParams;
};
static OP_INFO OpInfo[] = 
{
	{"Bigger",		Container::OP_BIGGER,			1,	1},
	{">",			Container::OP_BIGGER,			1,	1},

	{"BiggerEq",	Container::OP_BIGGER_EQ,		1,	1},
	{">=",			Container::OP_BIGGER_EQ,		1,	1},

	{"Smaller",		Container::OP_SMALLER,			1,	1},
	{"<",			Container::OP_SMALLER,			1,	1},

	{"SmallerEq",	Container::OP_SMALLER_EQ,		1,	1},
	{"<=",			Container::OP_SMALLER_EQ,		1,	1},

	{"EQ",			Container::OP_EQ,				1,	1},
	{"EQUAL",		Container::OP_EQ,				1,	1},
	{"=",			Container::OP_EQ,				1,	1},
	{"==",			Container::OP_EQ,				1,	1},

	{"DIFF",		Container::OP_DIFF,				1,	1},
	{"different",	Container::OP_DIFF,				1,	1},
	{"!=",			Container::OP_DIFF,				1,	1},
	{"<>",			Container::OP_DIFF,				1,	1},

	{"INSIDE",		Container::OP_INSIDE,			2,	2},
	{"OUTSIDE",		Container::OP_OUTSIDE,			2,	2},

	{"AND",			Container::OP_MASK,				1,	1},
	{"BITAND",		Container::OP_MASK,				1,	1},
	{"&",			Container::OP_MASK,				1,	1},

	{"ANDEQ",		Container::OP_MASK_EQ,			2,	2},
	{"&=",			Container::OP_MASK_EQ,			2,	2},

	{"IN",			Container::OP_IN,				1,	0xFFFF},
	{"OUT",			Container::OP_OUT,				1,	0xFFFF},
};
//============================================
int FInfoSort(FeatInfo &f1,FeatInfo &f2)
{
	if (f1.Key>f2.Key)
		return 1;
	if (f1.Key<f2.Key)
		return -1;
	return 0;
}
//=============================================
Container::Container()
{
	ObjectName = "Container";

	LinkPropertyToDouble("Label",Label,1,"Label value for Container records");
	LinkPropertyToString("FeatureInformationFile",FeatureInformationFile,"","File that contains informations about features to be extracted from container");

	AddCacheProperties();

}
bool Container::ProcessExpression(GML::Utils::GString &expr,FeatInfo &fi)
{
	GML::Utils::GString		opName,params,tok;
	int						pos = 0;
	UInt32					value;
	
	if (expr.SplitInTwo("(",&opName,&params)==false)
	{
		notifier->Error("[%s] -> Invalid format for %s. Expecting 'operation(param1,param2,...)'",ObjectName,expr.GetText());
		return false;
	}
	opName.Strip();
	params.Replace(")","");
	fi.Params = 0;
	fi.ParamsStartIndex = ParamValues.Len();

	while (params.CopyNext(&tok,",",&pos))
	{
		if (tok.ConvertToUInt32(&value)==false)
		{
			notifier->Error("[%s] -> Invalid numeric value (%s) in %s.",ObjectName,tok.GetText(),expr.GetText());
			return false;
		}
		if (ParamValues.PushByRef(value)==false)
		{
			notifier->Error("[%s] -> Unable to allocate memory for value (%s) in %s.",ObjectName,tok.GetText(),expr.GetText());
			return false;
		}
		fi.Params++;
	}

	for (UInt32 tr=0;tr<sizeof(OpInfo)/sizeof(OP_INFO);tr++)
	{
		if (opName.Equals(OpInfo[tr].Name,true))
		{
			if ((fi.Params<OpInfo[tr].MinParams) || (fi.Params>OpInfo[tr].MaxParams))
			{
				notifier->Error("[%s] -> Invalid number of parameters (%d) for operation '%s' in %s",ObjectName,fi.Params,expr.GetText());
				return false;
			}
			fi.Op = OpInfo[tr].ID;
			return true;
		}
	}
	notifier->Error("[%s] -> Unknown command (%s) in %s",ObjectName,opName.GetText(),expr.GetText());
	return false;
}
bool Container::LoadFIF()
{
	GML::Utils::GString		all,line;
	GML::Utils::GString		tok[5];
	int						pos_line=0,nr_tok;
	UInt32					cID,fID,tr;
	FeatInfo				fi;

	if (all.LoadFromFile(FeatureInformationFile.GetText())==false)
	{
		notifier->Error("[%s] -> Unable to open : %s",ObjectName,FeatureInformationFile.GetText());
		return false;
	}
	if (ParamValues.Create(1024)==false)
	{
		notifier->Error("[%s] -> Unable allocate memory for param values",ObjectName);
		return false;
	}
	FInfo.DeleteAll();
	while (all.CopyNextLine(&line,&pos_line))
	{
		line.Strip();
		if (line.Len()==0)
			continue;
		if (line.Split(";",tok,5,&nr_tok)==false)
		{
			notifier->Error("[%s] -> Invalid format for %s",ObjectName,line.GetText());
			return false;
		}
		if (nr_tok<4)
		{
			notifier->Error("[%s] -> To few field in %s. Format is (name,type,cID,fId,[expresion])",ObjectName,line.GetText());
			return false;
		}		
		fi.Op = OP_NONE;
		fi.ParamsStartIndex = 0;
		fi.Params = 0;
		fi.Type = GML::DB::TYPES::UNKNOWN;
		for (tr=0;tr<sizeof(FeatTypes)/sizeof(StructFeatTypes);tr++)
		{
			if (tok[1].Equals(FeatTypes[tr].Name,true))
			{
				fi.Type = FeatTypes[tr].Value;
				break;
			}
		}
		if (fi.Type == GML::DB::TYPES::UNKNOWN)
		{
			notifier->Error("[%s] -> Invalid type value (%s) in line %s",ObjectName,tok[1].GetText(),line.GetText());
			return false;
		}

		// verific ca cID si fID sa fie valide
		if (tok[2].ConvertToUInt32(&cID)==false)
		{
			notifier->Error("[%s] -> Invalid numeric value (%s) in line %s",ObjectName,tok[2].GetText(),line.GetText());
			return false;
		}
		if (tok[3].ConvertToUInt32(&fID)==false)
		{
			notifier->Error("[%s] -> Invalid numeric value (%s) in line %s",ObjectName,tok[3].GetText(),line.GetText());
			return false;
		}
		fi.Key = (((UInt64)cID)<<32) + ((UInt64)fID);
		if (nr_tok==5)
		{
			tok[4].Strip();
			if ((fi.Type!=GML::DB::TYPES::BOOLEAN) && (tok[4].Len()>0))
			{
				notifier->Error("[%s] -> Expresion (%s) can be only of type 'bool' in line %s",ObjectName,tok[4].GetText(),line.GetText());
				return false;
			}
			// procesez si expresia
			if (tok[4].Len()>0)
			{
				if (ProcessExpression(tok[4],fi)==false)
				{
					notifier->Error("[%s] -> Invalid expresion (%s) in line %s",ObjectName,tok[4].GetText(),line.GetText());
					return false;
				}				
			}
		}
		fi.ColumnIndex = Columns.Len();
		if (AddColumn(fi.Type,GML::DB::COLUMNTYPE::FEATURE,tok[0].GetText())==false)
			return false;
		if (FInfo.PushByRef(fi)==false)
		{
			notifier->Error("[%s] -> Unable to allocate memory for line %s",ObjectName,line.GetText());
			return false;
		}
	}
	if (FInfo.Len()==0)
	{
		notifier->Error("[%s] -> No features found in %s. Invalid format!",ObjectName,FeatureInformationFile.GetText());
		return false;
	}
	FInfo.Sort(FInfoSort);
	notifier->Info("[%s] -> Total features: %d , Total params: %d",ObjectName,FInfo.Len(),ParamValues.Len());
	return true;
}
bool Container::OnInit()
{
	Header	h;

	if (file.Open(fileName.GetText(),CacheSize)==false)
	{
		notifier->Error("[%s] -> Unable to open : %s",ObjectName,fileName.GetText());
		return false;
	}
	// adaug coloanele standard
	if (AddColumn(GML::DB::TYPES::HASH,GML::DB::COLUMNTYPE::HASH,"Hash")==false)
		return false;
	if (AddColumn(GML::DB::TYPES::DOUBLE,GML::DB::COLUMNTYPE::LABEL,"Label")==false)
		return false;	
	if (file.GetBuffer(0,&h,sizeof(h))==false)
	{
		notifier->Error("[%s] -> Unable to read header for %s",ObjectName,fileName.GetText());
		return false;
	}
	if (h.Magic!=0x544E4346)
	{
		notifier->Error("[%s] -> Invalid header in %s",ObjectName,fileName.GetText());
		return false;
	}
	if (h.nrRecords==0)
	{
		notifier->Error("[%s] -> NULL number of records in %s",ObjectName,fileName.GetText());
		return false;
	}
	// hederul e ok , 
	if (LoadFIF()==false)
		return false;

	nrRecords = h.nrRecords;	
	return true;
}
bool Container::Close()
{
	file.Close();
	return true;
}
bool Container::BeginIteration()
{
	CurentPos = 12;
	return true;
}
bool Container::OnReadNextRecord(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)
{
	UInt32		size,read;
	UInt8		id;
	UInt32		vIndex,fIndex,value,tr;
	UInt32		*params;
	FeatInfo	fi;
	FeatInfo	*cfi;
	int			found;

	VectPtr[1].Value.DoubleVal = Label;
	if (file.ReadUInt32(CurentPos,size)==false)
		return false;
	read = 0;
	while (read<size)
	{
		if (file.ReadUInt8(CurentPos,id)==false)
			return false;
		read++;
		fi.Key = id & 0x0F;
		fi.Key <<=32;
        fIndex = (id >> 4) & 3;
        vIndex = (id >> 6) & 3;		
		value = 0;
		if (file.ReadBuffer(CurentPos,&fi.Key,fIndex+1)==false)
			return false;
		read+=(fIndex+1);
		switch (vIndex)
		{
			case 0: 
				if (file.ReadBuffer(CurentPos,&value,1)==false)
					return false;
				read++;
				break;
			case 1:
				if (file.ReadUInt32(CurentPos,value)==false)
					return false;
				read+=sizeof(UInt32);
				break;
			case 2:
				value = 0;
				break;
			case 3:
				value = 0xFFFFFFFF;
				break;
		};
		// verificare pe MD5
		if ((fi.Key>=0xF00001388) && (fi.Key<=0xF0000138B))
		{
			VectPtr[0].Value.Hash.Hash.dwValue[fi.Key-0xF00001388] = value;
			continue;
		}		
		// analiza pe valoarea
		if ((found = FInfo.BinarySearch(fi,FInfoSort))>=0)
		{
			// setez valoarea
			cfi = FInfo.GetPtrToObject(found);
			params = ParamValues.GetPtrToObject(cfi->ParamsStartIndex);
			switch (cfi->Op)
			{
				case OP_NONE:
					if (cfi->Type == GML::DB::TYPES::BOOLEAN)
						VectPtr[cfi->ColumnIndex].Value.UInt32Val = value;
					else
						VectPtr[cfi->ColumnIndex].Value.BoolVal = true;
					break;
				case OP_BIGGER:
					if (value>(*params))
						VectPtr[cfi->ColumnIndex].Value.BoolVal = true;
					break;
				case OP_BIGGER_EQ:
					if (value>=(*params))
						VectPtr[cfi->ColumnIndex].Value.BoolVal = true;
					break;
				case OP_SMALLER:
					if (value<(*params))
						VectPtr[cfi->ColumnIndex].Value.BoolVal = true;
					break;
				case OP_SMALLER_EQ:
					if (value<=(*params))
						VectPtr[cfi->ColumnIndex].Value.BoolVal = true;
					break;
				case OP_EQ:
					if (value==(*params))
						VectPtr[cfi->ColumnIndex].Value.BoolVal = true;
					break;
				case OP_DIFF:
					if (value!=(*params))
						VectPtr[cfi->ColumnIndex].Value.BoolVal = true;
					break;
				case OP_INSIDE:
					if ((value>=params[0]) && (value<=params[1]))
						VectPtr[cfi->ColumnIndex].Value.BoolVal = true;
					break;
				case OP_OUTSIDE:
					if ((value<params[0]) || (value>params[1]))
						VectPtr[cfi->ColumnIndex].Value.BoolVal = true;
					break;
				case OP_MASK:
					if ((value & (*params))!=0)
						VectPtr[cfi->ColumnIndex].Value.BoolVal = true;
					break;
				case OP_MASK_EQ:
					if ((value & params[0])==params[1])
						VectPtr[cfi->ColumnIndex].Value.BoolVal = true;
					break;
				case OP_IN:
					for (tr=0;tr<cfi->Params;tr++,params++)
					{
						if (value==(*params))
						{
							VectPtr[cfi->ColumnIndex].Value.BoolVal = true;
							break;
						}
					}
					break;
				case OP_OUT:
					for (tr=0;tr<cfi->Params;tr++,params++)
					{
						if (value==(*params))						
							break;
					}
					if (tr==cfi->Params)
						VectPtr[cfi->ColumnIndex].Value.BoolVal = true;
					break;
				default:
					return false;
			}
		}
		
	}
	return true;
}

