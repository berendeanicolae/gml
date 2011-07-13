#include "HashWriter.h"

HashWriter::HashWriter()
{
	ObjectName = "HashWriter";

	//Add extra commands here
	SetPropertyMetaData("Command","!!LIST:None=0,GetNegative,GetPositive,GetAllIn1,GetAllSeparated!!");
	LinkPropertyToString("OutputFileName", OutputFileName, "", "FileName for the file to save hashes in");			
}
bool HashWriter::Init()
{
	if (InitConnections()==false)
		return false;
	if (InitThreads()==false)
		return false;
	if (SplitMLThreadDataRange(con->GetRecordCount())==false)
		return false;
	if (con->CreateMlRecord(MainRecord)==false)
	{
		notif->Error("[%s] -> Unable to create MainRecord",ObjectName);
		return false;
	}
	// Add extra initialization here
	return true;
}
void HashWriter::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand)
{
	switch (threadCommand)
	{
		case THREAD_COMMAND_NONE:
			// do nothing
			return;
		// add extra thread command processes here
	};
}
bool HashWriter::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	HashWriterThreadData	*obj_td = new HashWriterThreadData();
	if (obj_td==NULL)
		return false;
	// add extra initialization for obj_td

	thData.Context = obj_td;
	return true;
}
bool HashWriter::SaveHashes(UInt32 command)
{
	GML::Utils::File		f,fp,fn;
	GML::Utils::GString		fName,tempPos,tempNeg,hash;
	UInt32				tr,count = 0,countPozitive = 0,countNegative=0;
	GML::DB::RecordHash		rHash;
	double				Label;

	if (tempPos.Create(0x10000)==false)
	{
		notif->Error("[%s] -> Unable to alloc memory for cache",ObjectName);
		return false;
	}
	if (tempNeg.Create(0x10000)==false)
	{
		notif->Error("[%s] -> Unable to alloc memory for cache",ObjectName);
		return false;
	}

	if (command == COMMAND_SHOW_ALL_IN_2)
	{
		if (fp.Create(OutputFileNamePositive.GetText())==false)
		{
			notif->Error("[%s] -> Unable to create: %s",ObjectName,OutputFileNamePositive.GetText());
			return false;
		}
		if (fn.Create(OutputFileNameNegative.GetText())==false)
		{
			notif->Error("[%s] -> Unable to create: %s",ObjectName,OutputFileNameNegative.GetText());
			return false;
		}				
	} else {
		if (f.Create(OutputFileName.GetText())==false)
		{
			notif->Error("[%s] -> Unable to create: %s",ObjectName,OutputFileName.GetText());
			return false;
		}
	}

	for (tr=0;tr<con->GetRecordCount();tr++)
	{
		if (con->GetRecordLabel(Label,tr)==false)
		{
			notif->Error("[%s] -> Unable to read label for #%d",ObjectName,tr);
			return false;
		}
		if (con->GetRecordHash(rHash,tr)==false)
		{
			notif->Error("[%s] -> Unable to read record hash for #%d",ObjectName,tr);
			return false;
		}

		if (rHash.ToString(hash)==false)
		{
			notif->Error("[%s] -> Unable to convert record hash for #%d",ObjectName,tr);
			return false;
		}
		if (Label==-1 && command!=COMMAND_SHOW_POSITIVE)
		{
			if (tempNeg.AddFormated("%s\n",hash.GetText())==false)
			{
				notif->Error("[%s] -> Unable to create log entry for record #%d",ObjectName,tr);
				return false;
			}
		} else {
			if (Label==1 && command!=COMMAND_SHOW_NEGATIVE)
				if (tempPos.AddFormated("%s\n",hash.GetText())==false)
				{
					notif->Error("[%s] -> Unable to create log entry for record #%d",ObjectName,tr);
					return false;
				}
		}	
		count++;
		if (Label==1)
			countPozitive++;
		else
			countNegative++;
		if (tempPos.Len()>64000)
		{	
			if (command!=COMMAND_SHOW_ALL_IN_2)		
			{
				if (f.Write(tempPos,tempPos.Len())==false)
				{
					notif->Error("[%s] -> Unable to write to %s",ObjectName,OutputFileName.GetText());
					return false;
				}
				tempPos.Truncate(0);
				tempPos.Set("");
			} else {
				if (fp.Write(tempPos,tempPos.Len())==false)
				{
					notif->Error("[%s] -> Unable to write to %s",ObjectName,OutputFileNamePositive.GetText());
					return false;
				}
				tempPos.Truncate(0);
				tempPos.Set("");				
			}
		}
		if (tempNeg.Len()>64000)
		{	
			if (command!=COMMAND_SHOW_ALL_IN_2)		
			{
				if (f.Write(tempNeg,tempNeg.Len())==false)
				{
					notif->Error("[%s] -> Unable to write to %s",ObjectName,OutputFileName.GetText());
					return false;
				}
				tempNeg.Truncate(0);
				tempNeg.Set("");
			} else {
				if (fn.Write(tempNeg,tempNeg.Len())==false)
				{
					notif->Error("[%s] -> Unable to write to %s",ObjectName,OutputFileNameNegative.GetText());
					return false;
				}
				tempNeg.Truncate(0);
				tempNeg.Set("");				
			}
		}

	}
	if (command!=COMMAND_SHOW_ALL_IN_2)		
	{
		if (f.Write(tempPos,tempPos.Len())==false)
		{
			notif->Error("[%s] -> Unable to write to %s",ObjectName,OutputFileName.GetText());
			return false;
		}
		if (f.Write(tempNeg,tempNeg.Len())==false)
		{
			notif->Error("[%s] -> Unable to write to %s",ObjectName,OutputFileName.GetText());
			return false;
		}
		f.Close();
		if (command==COMMAND_SHOW_NEGATIVE) 
			notif->Info("[%s] -> %d records written in %s",ObjectName,countNegative,OutputFileName.GetText());
		else
			if (command==COMMAND_SHOW_POSITIVE)
				notif->Info("[%s] -> %d records written in %s",ObjectName,countPozitive,OutputFileName.GetText());
			else
				notif->Info("[%s] -> %d records written in %s",ObjectName,count,OutputFileName.GetText());				
		return true;
	} else {
		if (fp.Write(tempPos,tempPos.Len())==false)
		{
			notif->Error("[%s] -> Unable to write to %s",ObjectName,OutputFileNamePositive.GetText());
			return false;
		}
		if (fn.Write(tempNeg,tempNeg.Len())==false)
		{
			notif->Error("[%s] -> Unable to write to %s",ObjectName,OutputFileNameNegative.GetText());
			return false;
		}
		fp.Close();
		fn.Close();
		notif->Info("[%s] -> %d records written in %s",ObjectName,countPozitive,OutputFileNamePositive.GetText());
		notif->Info("[%s] -> %d records written in %s",ObjectName,countNegative,OutputFileNameNegative.GetText());
		return true;
	}
	return true;
}

void HashWriter::OnExecute()
{
	switch (Command)
	{
		case COMMAND_NONE:
			notif->Error("[%s] -> Nothing to do , select another command ",ObjectName);
			break;
		case COMMAND_SHOW_ALL_IN_2:
			OutputFileNamePositive.SetFormated("%s.Pos",OutputFileName.GetText());
			OutputFileNameNegative.SetFormated("%s.Neg",OutputFileName.GetText());
		case COMMAND_SHOW_NEGATIVE:
		case COMMAND_SHOW_POSITIVE:
		case COMMAND_SHOW_ALL_IN_1:
			SaveHashes(Command);
			break;
		default:
			notif->Error("[%s] -> Unknown command ID: %d",ObjectName,Command);
			break;
	}	
}
