#include "SimpleTextFileDB.h"

SimpleTextFileDB::SimpleTextFileDB()
{
	ObjectName = "SimpleTextFileDB";

	AddCacheProperties();

}
bool SimpleTextFileDB::OnInit()
{
	GML::Utils::GString		temp;
	int						tokPos;
	UInt64					pos=0,last;
	UInt32					value;
	
	DataStart = 0xFFFFFFFFFFFFFFFF;

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
	
	// vad cate recorduri si cel fel de coloane am
	nrRecords = 0;	
	notifier->StartProcent("[%s] -> Analizyng data base ",ObjectName);
	while (file.IsEOF(pos)==false)
	{
		last = pos;
		if (file.ReadLine(pos,line)==false)
		{
			notifier->Error("[%s] -> Unable to read line from file : %s",ObjectName,fileName.GetText());
			return false;
		}
		if (line.Len()==0)
			continue;
		line.Strip();
		if (DataStart == 0xFFFFFFFFFFFFFFFF)
		{
			if ((line.StartsWith("Feature",true)) && (line.Contains("=")))
			{
				if (line.SplitInTwo("=",NULL,&temp)==false)
				{
					notifier->Error("[%s] -> Internal error on : %s",ObjectName,line.GetText());
					return false;
				}
				temp.Strip();
				if (temp.Len()==0)
				{
					notifier->Error("[%s] -> Invalid feature name : %s",ObjectName,line.GetText());
					return false;
				}
				if (AddColumn(GML::DB::TYPES::DOUBLE,GML::DB::COLUMNTYPE::FEATURE,temp.GetText())==false)
					return false;
				continue;
			}
		}
		// verific daca e o linie normala
		if (line.Contains(":")==false)
		{
			notifier->Error("[%s] -> Invalid line : %s",ObjectName,line.GetText());
			return false;
		}
		if (line.SplitInTwo(":",NULL,&temp,-1)==false)
		{
			notifier->Error("[%s] -> Internal error on SplitInTwo for line : %s",ObjectName,line.GetText());
			return false;
		}
		tokPos = 0;
		while (temp.CopyNext(&token,",",&tokPos))
		{
			token.Strip();
			if (token.ConvertToUInt32(&value)==false)
			{
				notifier->Error("[%s] -> Invalid number (%s) in line: %s",ObjectName,token.GetText(),line.GetText());
				return false;
			}
			while (Columns.Len()<=value+2)
			{
				token.SetFormated("Feat_%d",Columns.Len()-2);
				if (AddColumn(GML::DB::TYPES::DOUBLE,GML::DB::COLUMNTYPE::FEATURE,token.GetText())==false)
					return false;				
			}
		}
		if (DataStart == 0xFFFFFFFFFFFFFFFF)
			DataStart = last;
		nrRecords++;
		if ((nrRecords % 1000)==0)
			notifier->SetProcent((double)pos,(double)file.GetFileSize());
	}
	notifier->EndProcent();
	notifier->Info("[%s] -> Feature count: %d,  Records count: %d",ObjectName,Columns.Len()-2,nrRecords);
	notifier->Info("[%s] -> Data starts : %d ",ObjectName,(UInt32)DataStart);
	return true;
}
bool SimpleTextFileDB::Close()
{
	file.Close();
	Columns.Free();
	Names.Free();
	return true;
}
bool SimpleTextFileDB::BeginIteration()
{
	CurentPos = DataStart;
	if (DataStart == 0xFFFFFFFFFFFFFFFF)
		return false;
	return true;
}
bool SimpleTextFileDB::OnReadNextRecord(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)
{
	GML::DB::DBRecord*	r;
	char*				ptr;
	char				temp[64];
	UInt32				cPoz,index,columnsCount;

	columnsCount = Columns.Len();
	do
	{
		if (file.ReadLine(CurentPos,line)==false)
		{
			notifier->Error("[%s] -> Unable to read a line from location (%ul) ",ObjectName,CurentPos);
			return false;
		}
		line.Strip();
	} while (line.Len()==0);
	
	// fill up

	r = VectPtr.GetPtrToObject(0);
	ptr = line.GetText();
	cPoz = 0;	
	for (;(*ptr)!=0;ptr++)
	{
		if (((*ptr)==' ') || ((*ptr)=='\t'))
			continue;
		if (
			(((*ptr)>='0') && ((*ptr)<='9')) || 
			(((*ptr)>='a') && ((*ptr)<='f')) ||
			(((*ptr)>='A') && ((*ptr)<='F')) ||
			((*ptr)=='.') ||
			((*ptr)=='-') ||
			((*ptr)=='+') ||
			((*ptr)=='[') || 
			((*ptr)==']')
			) 
		{
			temp[cPoz] = (*ptr);
			cPoz++;
			if (cPoz==63)
			{
				temp[cPoz] = 0;
				notifier->Error("[%s] -> Invalid word (%s) on line: %s ",ObjectName,temp,line.GetText());
				return false;
			}
			continue;
		}
		if ((*ptr)==':')
		{
			temp[cPoz] = 0;			
			// hash
			if ((cPoz==34) && (temp[0]=='[') && (temp[33]==']'))
			{
				temp[33]=0;
				if (r[0].Value.Hash.CreateFromText(&temp[1])==false)
				{
					notifier->Error("[%s] -> Invalid hash (%s) on line: %s ",ObjectName,&temp[1],line.GetText());
					return false;
				}
				cPoz = 0;
				continue;
			}
			if ((cPoz==2) && (temp[0]=='[') && (temp[1]==']'))
			{
				cPoz = 0;
				continue;
			}
			// label
			if (GML::Utils::GString::ConvertToDouble(temp,&r[1].Value.DoubleVal,cPoz)==false)
			{
				notifier->Error("[%s] -> Invalid label value (%s) on line: %s ",ObjectName,temp,line.GetText());
				return false;
			}
			cPoz = 0;
			continue;
		}
		if ((*ptr)==',')
		{
			temp[cPoz] = 0;			
			if (GML::Utils::GString::ConvertToUInt32(temp,&index,0,cPoz)==false)
			{
				notifier->Error("[%s] -> Invalid number value (%s) on line: %s ",ObjectName,temp,line.GetText());
				return false;
			}
			index+=2;			
			if (index>=columnsCount)
			{
				notifier->Error("[%s] -> Invalid index (%d) on line: %s ",ObjectName,index,line.GetText());
				return false;
			}
			r[index].Value.DoubleVal = 1.0;
			cPoz=0;
			continue;
		}
		notifier->Error("[%s] -> Invalid character (0x%02X) on line: %s ",ObjectName,(UInt32)(*ptr),line.GetText());
		return false;
	}
	temp[cPoz] = 0;
	cPoz=0;
	if (temp[0]!=0)
	{
		if (GML::Utils::GString::ConvertToUInt32(temp,&index)==false)
		{
			notifier->Error("[%s] -> Invalid number value (%s) on line: %s ",ObjectName,temp,line.GetText());
			return false;
		}
		index+=2;
		if (index>=columnsCount)
		{
			notifier->Error("[%s] -> Invalid index (%d) on line: %s ",ObjectName,index,line.GetText());
			return false;
		}
		r[index].Value.DoubleVal = 1.0;
	}
	// analiza pe linie
	return true;
}
