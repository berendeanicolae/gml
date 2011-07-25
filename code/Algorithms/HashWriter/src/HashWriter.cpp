#include "HashWriter.h"

int FeatInfoCompare(FeatInfo &f1,FeatInfo &f2)
{
	return memcmp(f1.fHash.Hash.bValue,f2.fHash.Hash.bValue,16);
}
//============================================================================== 
HashWriter::HashWriter()
{
	ObjectName = "HashWriter";

	//Add extra commands here
	SetPropertyMetaData("Command","!!LIST:None=0,GetNegative,GetPositive,GetAllIn1,GetAllSeparated,SaveAll,GroupHashesByFeatures!!");
	LinkPropertyToString("OutputFileName", OutputFileName, "", "FileName for the file to save hashes in");		
	LinkPropertyToBool("SaveFeaturesNames", SaveFeaturesNames, false, "Save also feaures names");
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
bool HashWriter::SaveAll()
{
	GML::Utils::File		f;
	GML::Utils::GString		temp,hash,curent;
	UInt32					tr,gr;

	if (temp.Create(0x1000)==false)
	{
		notif->Error("[%s] -> Unable to alloc memory for cache",ObjectName);
		return false;
	}
	if (f.Create(OutputFileName.GetText())==false)
	{
		notif->Error("[%s] -> Unable to create: %s",ObjectName,OutputFileName.GetText());
		return false;
	}

	temp.Set("");
	notif->Info("[%s] -> Saving %d records to %s",ObjectName,con->GetRecordCount(),OutputFileName.GetText());
	for (tr=0;tr<con->GetRecordCount();tr++)	
	{
		if (con->GetRecord(MainRecord,tr,GML::ML::ConnectorFlags::STORE_HASH)==false)
		{
			notif->Error("[%s] -> Unable to read record #%d",ObjectName,tr);
			return false;
		}
		if (MainRecord.Hash.ToString(hash)==false)
		{
			notif->Error("[%s] -> Unable to convert record hash for #%d",ObjectName,tr);
			return false;
		}
		if (temp.AddFormated("%s|%lf|",hash.GetText(),MainRecord.Label)==false)
		{
			notif->Error("[%s] -> Unable to create line ",ObjectName);
			return false;
		}
		for (gr=0;gr<con->GetFeatureCount();gr++)
		{
			if (((double)((int)MainRecord.Features[gr])) == MainRecord.Features[gr])
			{
				if (temp.AddFormated("%d|",(int)MainRecord.Features[gr])==false)
				{
					notif->Error("[%s] -> Unable to create line for feature #%d",ObjectName,gr);
					return false;
				}
			} else {
				if (temp.AddFormated("%lf|",MainRecord.Features[gr])==false)
				{
					notif->Error("[%s] -> Unable to create line for feature #%d",ObjectName,gr);
					return false;
				}			
			}
		}
		temp.Add("\n");		
		if (temp.Len()>64000)
		{						
			if (f.Write(temp.GetText(),temp.Len())==false)
			{
				notif->Error("[%s] -> Unable to write to %s",ObjectName,OutputFileName.GetText());
				return false;
			}
			temp.Truncate(0);
			temp.Set("");
		}
	}
	if (temp.Len()>0)
	{			
		if (f.Write(temp.GetText(),temp.Len())==false)
		{
			notif->Error("[%s] -> Unable to write to %s",ObjectName,OutputFileName.GetText());
			return false;
		}
	}
	f.Close();
	notif->Info("[%s] -> %s saved ok !",ObjectName,OutputFileName.GetText());
	return true;
}
void HashWriter::PrintFlist()
{
	GML::Utils::GString		tmpR,tmpF;
	UInt32					tr;

	for (tr=0; tr<FList.Len(); tr++)
	{		
		FList[tr].rHash.ToString(tmpR);
		FList[tr].fHash.ToString(tmpF);
		notif->Info("[%s] -> %d %s %s %f",ObjectName,FList[tr].Index, tmpR.GetText(), tmpF.GetText(), FList[tr].Label);
	}
}
bool HashWriter::LoadRecords()
{
	UInt32		tr,featSize;

	if (FList.Create(con->GetRecordCount(),true)==false)
	{
		notif->Error("[%s] -> Unable to allocate memory for !",ObjectName);
		return false;
	}
	for (tr=0;tr<FList.Len();tr++)
		FList[tr].Index = tr;

	featSize = con->GetFeatureCount() * sizeof(double);
	notif->StartProcent("[%s] -> Computing hashes ... ",ObjectName);
	for (tr=0;tr<FList.Len();tr++)
	{
		if (con->GetRecord(MainRecord,tr)==false)
		{
			notif->Error("[%s] -> Unable to read record #%d!",ObjectName,tr);
			return false;
		}
		if (FList[tr].fHash.ComputeHashForBuffer(MainRecord.Features,featSize)==false)
		{
			notif->Error("[%s] -> Unable to compute features hash for record #%d!",ObjectName,tr);
			return false;
		}		
		if (con->GetRecordHash(FList[tr].rHash,tr)==false)
		{
			notif->Error("[%s] -> Unable to read record hash for #%d",ObjectName,tr);
			return false;
		}
		FList[tr].Label = MainRecord.Label;
		if ((tr % 1000)==0)
			notif->SetProcent(tr,FList.Len());
	}
	notif->EndProcent();

	return true;
}
bool HashWriter::GetFeatures(GML::Utils::GString &features, UInt32 recIndex)
{
	UInt32					tr;
	GML::Utils::GString		featName;

	features.Set("------------------------------------------\n");
	if (con->GetRecord(MainRecord,recIndex)==false)
	{
		notif->Error("[%s] -> Unalbe to read record %d!",ObjectName,recIndex);
		return false;
	}
	for (tr=0;tr<con->GetFeatureCount();tr++)
	{
		if ((double)(MainRecord.Features[tr])!=0.0)
		{
			if (con->GetFeatureName(featName, tr) == false)
			{
				notif->Error("[%s] -> Unable to get feature name for feature with index: %d",ObjectName, tr);
				return false;
			}		
			features.AddFormated("%s\n",featName.GetText());
		}
	}
	features.Add("------------------------------------------");
	return true;
}
bool HashWriter::SaveHashGroupsByFeatComb()
{	
	UInt32					tr,featSize,posCount,negCount;
	GML::Utils::GString		temp,hash;
	GML::Utils::File		f;
	GML::DB::RecordHash		tempFeatHash;

	if(LoadRecords()==false)
	{
		notif->Error("[%s] -> Failed to read records!",ObjectName);
		return false;
	}
	//PrintFlist();	
	// am citit toate datele , le sortez
	notif->Info("[%s] -> Sorting ... ",ObjectName);	
	//notif->Info("================================================");
	FList.Sort(FeatInfoCompare);
	//PrintFlist();

	if (temp.Create(0x1000)==false)
	{
		notif->Error("[%s] -> Unable to alloc memory for cache",ObjectName);
		return false;
	}
	if (f.Create(OutputFileName.GetText())==false)
	{
		notif->Error("[%s] -> Unable to create: %s",ObjectName,OutputFileName.GetText());
		return false;
	}

	temp.Set("");
	tempFeatHash = FList[0].fHash;
	posCount = negCount = 0;
	notif->Info("[%s] -> Saving %d records to %s",ObjectName,con->GetRecordCount(),OutputFileName.GetText());
	for (tr=0;tr<con->GetRecordCount();tr++)	
	{
		if (memcmp(FList[tr].fHash.Hash.bValue,tempFeatHash.Hash.bValue,16)!=0)
		{
			if (SaveFeaturesNames)
			{
				if (GetFeatures(hash,FList[tr].Index)==false)
					return false;
				if (temp.AddFormated("%s\n",hash.GetText())==false)
				{
					notif->Error("[%s] -> Unable to add line ",ObjectName);
					return false;
				}
			}
			if (temp.AddFormated("T:%d|PC:%d|NC:%d\n================================================\n",posCount+negCount,posCount,negCount)==false)
			{
				notif->Error("[%s] -> Unable to add line ",ObjectName);
				return false;
			}
			posCount = negCount = 0;
			tempFeatHash = FList[tr].fHash;
		}
		if (FList[tr].rHash.ToString(hash)==false)
		{
			notif->Error("[%s] -> Unable to convert record hash for #%d",ObjectName,tr);
			return false;
		}		
		if (FList[tr].Label==1.0)
			posCount++;
		else
			negCount++;
		if (temp.AddFormated("%s|%lf",hash.GetText(),FList[tr].Label)==false)
		{
			notif->Error("[%s] -> Unable to create line ",ObjectName);
			return false;
		}		
		temp.Add("\n");		
		if (temp.Len()>64000)
		{						
			if (f.Write(temp.GetText(),temp.Len())==false)
			{
				notif->Error("[%s] -> Unable to write to %s",ObjectName,OutputFileName.GetText());
				return false;
			}
			temp.Truncate(0);
			temp.Set("");
		}
	}
	if (temp.Len()>0)
	{			
		if (f.Write(temp.GetText(),temp.Len())==false)
		{
			notif->Error("[%s] -> Unable to write to %s",ObjectName,OutputFileName.GetText());
			return false;
		}
	}
	f.Close();
	notif->Info("[%s] -> %s saved ok !",ObjectName,OutputFileName.GetText());
	
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
		case COMMAND_SAVE_ALL:
			SaveAll();
			break;
		case COMMAND_FEAT_COMB_GROUPS:
			SaveHashGroupsByFeatComb();
			break;
		default:
			notif->Error("[%s] -> Unknown command ID: %d",ObjectName,Command);
			break;
	}	
}
