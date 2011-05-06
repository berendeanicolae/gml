#include "CSV.h"

struct TypeNames
{
	char	*Name;
	UInt32	EnumValue;
};


TypeNames tNames[]=
{
	{"boolean",GML::DB::TYPES::BOOLEAN},
	{"bool",GML::DB::TYPES::BOOLEAN},
	{"b",GML::DB::TYPES::BOOLEAN},

	{"int8",GML::DB::TYPES::INT8},
	{"int16",GML::DB::TYPES::INT16},
	{"int32",GML::DB::TYPES::INT32},
	{"int",GML::DB::TYPES::INT32},
	{"int64",GML::DB::TYPES::INT64},

	{"i8",GML::DB::TYPES::INT8},
	{"i16",GML::DB::TYPES::INT16},
	{"i32",GML::DB::TYPES::INT32},
	{"i64",GML::DB::TYPES::INT64},


	{"uint8",GML::DB::TYPES::UINT8},
	{"uint16",GML::DB::TYPES::UINT16},
	{"uint32",GML::DB::TYPES::UINT32},
	{"uint",GML::DB::TYPES::UINT32},
	{"uint64",GML::DB::TYPES::UINT64},

	{"u8",GML::DB::TYPES::UINT8},
	{"u16",GML::DB::TYPES::UINT16},
	{"u32",GML::DB::TYPES::UINT32},	
	{"u64",GML::DB::TYPES::UINT64},


	{"float",GML::DB::TYPES::FLOAT},
	{"double",GML::DB::TYPES::DOUBLE},
	{"numeric",GML::DB::TYPES::DOUBLE},

	{"f",GML::DB::TYPES::FLOAT},
	{"d",GML::DB::TYPES::DOUBLE},
	{"n",GML::DB::TYPES::DOUBLE},

	{"hash",GML::DB::TYPES::HASH},
	{"h",GML::DB::TYPES::HASH},
};

CSV::CSV()
{
	ObjectName = "CSV";
	LinkPropertyToString("FileName",fileName,"","File that contains the database !");
}
bool	CSV::OnInit()
{
	return true;
}
bool	CSV::Connect ()
{
	GML::Utils::GString		token,tip;
	int						poz,poz2,tipPoz,tr,countUnk;
	Column					c;

	poz = 0;
	Columns.DeleteAll();
	if (allDB.LoadFromFile(fileName.GetText())==false)
	{
		notifier->Error("[%s] -> Unable to open (%s) for reading !",ObjectName,fileName.GetText());
		return false;
	}
	if (allDB.CopyNextLine(&tempStr,&poz)==false)
	{
		notifier->Error("[%s] -> Error reading header from %s",ObjectName,fileName.GetText());
		return false;
	}
	tempStr.Strip();
	// 
	poz2 = 0;
	countUnk = 0;
	while (tempStr.CopyNext(&token,",",&poz2))
	{
		token.Strip();
		tipPoz = token.Find("{");
		c.Type = GML::DB::TYPES::UNKNOWN;
		if (tipPoz>=0)
		{
			tip.Set(&token.GetText()[tipPoz+1]);
			tip.Replace("}","");
			tip.Strip();
			token.Truncate(tipPoz);
			token.Strip();
			for (tr=0;tr<sizeof(tNames)/sizeof(TypeNames);tr++)
			{
				if (tip.Equals(tNames[tr].Name,true))
				{
					c.Type = tNames[tr].EnumValue;
					break;
				}
			}
			if (c.Type == GML::DB::TYPES::UNKNOWN)
			{
				notifier->Error("[%s] -> Unknown type (%s) for column (%s) in %s",ObjectName,tip.GetText(),token.GetText(),tempStr.GetText());
				return false;
			}
		} else {
			countUnk++;
		}
		GML::Utils::GString::Set(c.Name,token.GetText(),MAX_COLUMN_NAME);
		

		if (Columns.PushByRef(c)==false)
		{
			notifier->Error("[%s] -> Unable to add '%s' to column names !",ObjectName,token.GetText());
			return false;
		}
	}
	if (Columns.Len()==0)
	{
		notifier->Error("[%s] -> first line in a csv file should be the header (found nothing) -> %s",ObjectName,tempStr.GetText());
		return false;
	}
	nrRecords = 0;
	while (allDB.CopyNextLine(&tempStr,&poz))
	{
		tempStr.Strip();
		if (tempStr.Len()>0)
			nrRecords++;
	}
	if (nrRecords==0)
	{
		notifier->Error("[%s] -> Missing records in %s",ObjectName,fileName.GetText());
		return false;
	}
	// daca am macar unul necunoscut , citesc prima linie ca sa fac un update
	if (countUnk>0)
	{
		if (ExecuteQuery("select * from table",NULL)==false)
			return false;
		GML::Utils::GTFVector<GML::DB::DBRecord> VectPtr;
		if (FetchNextRow(VectPtr)==false)
			return false;

	}


	notifier->Info("[%s] -> CSV file loaded : %d records",ObjectName,nrRecords);
	return true;
}
bool	CSV::Disconnect ()
{
	return true;
}
bool	CSV::ExecuteQuery (char* Statement,UInt32 *rowsCount)
{
	GML::Utils::GString		tempStr;
	unsigned int			limit;
	// daca se cere numarul de inregistrari
	if (GML::Utils::GString::Find(Statement,"count",true)>=0)
	{
		modQueryCount = true;
		if (rowsCount)
			(*rowsCount) = 1;
		return true;
	}
	if (GML::Utils::GString::EndsWith(Statement,"limit 1",true))
	{
		limit = 0;
		if (rowsCount)
			(*rowsCount) = 1;
	} else {
		if (GML::Utils::GString::Find(Statement,"limit",true)>=0)
		{
			tempStr.Set(&Statement[GML::Utils::GString::Find(Statement,"limit",true)+5]);
			tempStr.Strip();
			if (tempStr.Contains(","))
				tempStr.Truncate(tempStr.Find(","));
			tempStr.Strip();
			if (tempStr.ConvertToUInt32(&limit)==false)
			{
				notifier->Error("[%s] -> Invalid number for limit: (%s) in '%s'",ObjectName,tempStr.GetText(),Statement);
				return false;
			}
		} else { 
			limit = 0; 
		}
	}
	modQueryCount = false;
	if (limit==0)
	{
		dbPoz = 0;
		// skipez prima linie (hederul)
		for (int tr=0;tr<1;tr++)
		{
			if (allDB.CopyNextLine(&tempStr,&dbPoz)==false)
			{
				notifier->Error("[%s] -> File::Error reading a line from %s",ObjectName,fileName.GetText());
				return false;
			}
		}
		cIndex = 0;
	} else {
		if (cIndex!=limit)
		{
			notifier->Error("[%s] -> Invalid position (current = %d , requested = %d) in '%s'",ObjectName,cIndex,limit,Statement);
			return false;
		}
	}
	if (rowsCount)
		(*rowsCount) = nrRecords;
	// totul e ok
	return true;
}
bool    CSV::GetColumnInformations(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)
{
	GML::DB::DBRecord	rec;

	VectPtr.DeleteAll();

	for (UInt32 tr=0;tr<Columns.Len();tr++)
	{
		rec.Name = Columns[tr].Name;
		rec.Type = Columns[tr].Type;
		if (VectPtr.PushByRef(rec)==false)
		{
			notifier->Error("[%s] -> Unable to add column (%s) to Columns vectors",ObjectName,rec.Name);
			return false;
		}
	}	
	return true;
}
bool	CSV::UpdateValue(GML::Utils::GString &str,GML::DB::DBRecord &rec)
{
	if (rec.Type == GML::DB::TYPES::UNKNOWN)
	{
		// verific daca e bool
		if ((str.Equals("true",true)) || (str.Equals("t",true)) || (str.Equals("yes",true)))
		{
			rec.Type = GML::DB::TYPES::BOOLEAN;
			rec.Value.BoolVal = true;
			return true;
		}
		if ((str.Equals("false",true)) || (str.Equals("f",true)) || (str.Equals("no",true)))
		{
			rec.Type = GML::DB::TYPES::BOOLEAN;
			rec.Value.BoolVal = false;
			return true;
		}
		// verific daca e numar
		if (str.ConvertToDouble(&rec.Value.DoubleVal)==true)
		{
			rec.Type = GML::DB::TYPES::DOUBLE;
			return true;
		}
		// verific daca nu cumva e hash
		if (rec.Value.Hash.CreateFromText(str.GetText())==true)
		{
			rec.Type = GML::DB::TYPES::HASH;
			return true;
		}
		// altfel e string	
		return false;
	} 
	// daca am deja un tip setat
	switch (rec.Type)
	{
		case GML::DB::TYPES::BOOLEAN:
			rec.Value.BoolVal = (bool)((str.Equals("true",true)) || (str.Equals("t",true)) || (str.Equals("yes",true)));
			return true;			
		case GML::DB::TYPES::INT8:
			return str.ConvertToInt8(&rec.Value.Int8Val);
		case GML::DB::TYPES::INT16:
			return str.ConvertToInt16(&rec.Value.Int16Val);
		case GML::DB::TYPES::INT32:
			return str.ConvertToInt32(&rec.Value.Int32Val);
		case GML::DB::TYPES::INT64:
			return str.ConvertToInt64(&rec.Value.Int64Val);

		case GML::DB::TYPES::UINT8:
			return str.ConvertToUInt8(&rec.Value.UInt8Val);
		case GML::DB::TYPES::UINT16:
			return str.ConvertToUInt16(&rec.Value.UInt16Val);
		case GML::DB::TYPES::UINT32:
			return str.ConvertToUInt32(&rec.Value.UInt32Val);
		case GML::DB::TYPES::UINT64:
			return str.ConvertToUInt64(&rec.Value.UInt64Val);

		case GML::DB::TYPES::FLOAT:
			return str.ConvertToFloat(&rec.Value.FloatVal);
		case GML::DB::TYPES::DOUBLE:
			return str.ConvertToDouble(&rec.Value.DoubleVal);

		case GML::DB::TYPES::HASH:
			return rec.Value.Hash.CreateFromText(str.GetText());

	}
	return false;
}
bool	CSV::FetchNextRow (GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)
{
	GML::DB::DBRecord	rec;
	int					poz,tr;
	//UInt32				index;

	memset(&rec,0,sizeof(rec));

	if (VectPtr.DeleteAll()==false)
	{
		notifier->Error("[%s] -> Unable to delete all indexes from record vector",ObjectName);
		return false;
	}

	if (modQueryCount)
	{
		rec.Type = GML::DB::TYPES::UINT32;
		rec.Value.UInt32Val = nrRecords;
		if (VectPtr.PushByRef(rec)==false)
		{
			notifier->Error("[%s] -> Unable to add record count to vector !",ObjectName);
			return false;
		}
		return true;
	}

	while (true)
	{
		if (allDB.CopyNextLine(&tempStr,&dbPoz)==false)
			return false;
		tempStr.Strip();
		if (tempStr.Len()==0)
			continue;
		break;
	}
	
	cIndex++;
	poz = tr = 0;
	while (tempStr.CopyNext(&featureIndex,",",&poz))
	{
		if (tr>=Columns.Len())
		{
			notifier->Error("[%s] -> Too many values (%s)",ObjectName,tempStr.GetText());
		}
		featureIndex.Strip();
		rec.Name = Columns[tr].Name;
		rec.Type = Columns[tr].Type;
		// pun si valorile
		
		if (UpdateValue(featureIndex,rec)==false)
		{
			notifier->Error("[%s] -> Invalid format for '%s' in '%s'",ObjectName,featureIndex,tempStr.GetText());
			return false;
		}
		if (VectPtr.PushByRef(rec)==false)
		{
			notifier->Error("[%s] -> Unable to add record count to vector !",ObjectName);
			return false;
		}
		Columns[tr].Type = rec.Type;
		tr++;	
	}
	if (tr!=Columns.Len())
	{
		notifier->Error("[%s] -> Incomplete list of values (%s)",ObjectName,tempStr.GetText());
	}
	return true;
}

bool	CSV::InsertRow (char* Table, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect)
{
	notifier->Error("[%s] -> InsertRow function not suported !",ObjectName);
	return false;
}
bool	CSV::InsertRow (char* Table, char* Fields, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect)
{
	notifier->Error("[%s] -> InsertRow function not suported !",ObjectName);
	return false;
}
bool	CSV::Update (char* SqlStatement, GML::Utils::GTFVector<GML::DB::DBRecord> &WhereVals, GML::Utils::GTFVector<GML::DB::DBRecord> &UpdateVals)
{
	notifier->Error("[%s] -> Update function not suported !",ObjectName);
	return false;
}