#include "FeaturesStatistics.h"
#include <math.h>

void ThreadRedirectFunction(GML::Utils::IParalelUnit *paralel,void *context)
{
	FeaturesStatistics *fs = (FeaturesStatistics *)context;
	fs->OnRunThreadCommand(fs->fData[paralel->GetID()],paralel->GetCodeID());
}
//====================================================================================================
int  Compare_FeaturesInformations(FeaturesInformations &f1,FeaturesInformations &f2,void *context)
{
	UInt32	column = *((UInt32 *)context);


	if (f1.fnValue[column]>f2.fnValue[column])
		return 1;
	if (f1.fnValue[column]<f2.fnValue[column])
		return -1;
	return 0;
}
bool FeaturesInformations::operator< (FeaturesInformations &a)
{
	return (bool)(Index<a.Index);
}
bool FeaturesInformations::operator> (FeaturesInformations &a)
{
	return (bool)(Index>a.Index);
}
//====================================================================================================
Stats::Stats()
{
	fnCompute = NULL;
}
Stats::Stats(Stats &ref)
{
	fnCompute = ref.fnCompute;
	this->Name.Set(ref.Name.GetText());
}
//====================================================================================================
double Compute_RapPozNeg(FeaturesInformations *f)
{
	if (f->countPozitive>=f->countNegative)
		return f->countPozitive/f->countNegative;
	else
		return -(f->countNegative/f->countPozitive);
}
double Compute_ProcDiff(FeaturesInformations *f)
{
	return ((f->countPozitive*100)/f->totalPozitive)-((f->countNegative*100)/f->totalNegative);
}
double Compute_Diff(FeaturesInformations *f)
{
	return f->countPozitive-f->countNegative;
}
double Compute_AbsDiff(FeaturesInformations *f)
{
	if (f->countPozitive>f->countNegative)
		return f->countPozitive-f->countNegative;
	else
		return f->countNegative-f->countPozitive;
}
double Compute_F1(FeaturesInformations *f)
{
	double t_mal = f->countPozitive;
	double f_mal = f->totalPozitive - t_mal;
	double t_clean = f->countNegative;
	double f_clean = f->totalNegative - t_clean;

	double all_mal = f->totalPozitive;
	double all_clean = f->totalNegative;
	double miu_pl = (double)t_mal / all_mal;
	double miu_min = (double)t_clean / all_clean;
	double sigma_pl = sqrt((double)(t_mal * (1 - miu_pl) * (1 - miu_pl) + f_mal * miu_pl * miu_pl));
	double sigma_min = sqrt((double)(t_clean * (1 - miu_min) * (1 - miu_min) + f_clean * miu_min * miu_min));
	double v1 = miu_pl - miu_min;
	double v2 = sigma_pl + sigma_min;
	//if (v1 < 0) v1 = -v1;
	if (v2 < 0) v2 = -v2;
	if (t_mal + t_clean == 0) return 0;
	return v1 / v2;
	//double precision = (double)(f.t_mal) / ((double)f.t_mal);
}
double Compute_F2(FeaturesInformations *f)
{
	double t_mal = f->countPozitive;
	double f_mal = f->totalPozitive - t_mal;
	double t_clean = f->countNegative;
	double f_clean = f->totalNegative - t_clean;

	double all_mal = f->totalPozitive;
	double all_clean = f->totalNegative;
    double miu_pl = (double)t_mal / all_mal;
    double miu_min = (double)t_clean / all_clean;
    double miu_total = (double)(t_mal + t_clean) / (all_mal + all_clean);
    double sigma_pl = sqrt((double)(t_mal * (1 - miu_pl) * (1 - miu_pl) + f_mal * miu_pl * miu_pl));
    double sigma_min = sqrt((double)(t_clean * (1 - miu_min) * (1 - miu_min) + f_clean * miu_min * miu_min));
    double v1 = (miu_pl - miu_total) * (miu_pl - miu_total) + (miu_min - miu_total) * (miu_min - miu_total);
    double v2 = sigma_pl*sigma_pl + sigma_min*sigma_min;
    if (t_mal + t_clean == 0) return 0;
    return v1 / v2;
}
double Compute_ProcTo100(FeaturesInformations *f)
{
	if (f->countPozitive>f->countNegative)
		return 100-(f->countNegative/f->countPozitive)*100;
	if (f->countPozitive<f->countNegative)
		return -(100-(f->countPozitive/f->countNegative)*100);
	return 0;
}

//====================================================================================================

FeaturesStatistics::FeaturesStatistics()
{
	GML::Utils::GString		tmp;
	UInt32					tr;

	ObjectName = "FeaturesStatistics";

	LinkPropertyToString("DataBase"					,DataBase				,"");
	LinkPropertyToString("Connector"				,Conector				,"");
	LinkPropertyToString("Notifier"					,Notifier				,"");
	LinkPropertyToUInt32("ThreadsCount"				,threadsCount			,1);
	LinkPropertyToUInt32("ColumnWidth"				,columnWidth			,12,"Sets the column width (0 for no aligniation)");
	LinkPropertyToString("ResultFile"				,ResultFile				,"","Name of the file to save the result table or none if no save is requared");
	LinkPropertyToBool  ("NotifyResult"				,notifyResults			,true);
	LinkPropertyToDouble("MultiplyFactor"			,multiplyFactor			,1.0);
	

	AddNewStatFunction("Poz/Neg",Compute_RapPozNeg);
	AddNewStatFunction("ProcDiff",Compute_ProcDiff);
	AddNewStatFunction("Diff",Compute_Diff);
	AddNewStatFunction("Abs(Diff)",Compute_AbsDiff);
	AddNewStatFunction("F1",Compute_F1);
	AddNewStatFunction("F2",Compute_F2);
	AddNewStatFunction("ProcTo100",Compute_ProcTo100);

	SortProps.Set("!!LIST:NoSort=0xFFFF");
	WeightFileType.Set("!!LIST:None=0xFFFF");
	for (tr=0;tr<StatsData.Len();tr++)
	{
		char *text = StatsData[tr].Name.GetText();
		SortProps.AddFormated(",%s=%d",text,tr);
		WeightFileType.AddFormated(",%s=%d",StatsData[tr].Name.GetText(),tr);
	}
	SortProps.Add("!!");
	WeightFileType.Add("!!");

	LinkPropertyToUInt32("SortBy"					,sortBy					,0xFFFF,SortProps.GetText());
	LinkPropertyToUInt32("SortDirection"			,sortDirection			,0,"!!LIST:Ascendet=0,Descendent!!");
	LinkPropertyToUInt32("SaveFeaturesWeight"		,saveFeatureWeightFile	,0xFFFF,WeightFileType.GetText());
	LinkPropertyToString("FeaturesWeightFile"		,FeaturesWeightFile		,"");
}
bool FeaturesStatistics::AddNewStatFunction(char *name,double (*_fnCompute) ( FeaturesInformations *info))
{
	Stats	tmp;

	if (tmp.Name.Set(name)==false)
		return false;
	tmp.fnCompute = _fnCompute;
	return StatsData.PushByRef(tmp);
}
bool FeaturesStatistics::CreateFeaturesInfo(FeaturesThreadData *fInfo)
{
	UInt32	tr;
	if (con->CreateMlRecord(fInfo->Record)==false)
	{
		notif->Error("[%s] -> Unable to create MLRecord",ObjectName);
		return false;
	}

	if ((fInfo->FI = new FeaturesInfo[con->GetFeatureCount()])==NULL)
	{
		notif->Error("[%s] -> Unable to allocate space for FeaturesInfo (%d)",ObjectName,con->GetFeatureCount());
		return false;
	}
	for (tr=0;tr<con->GetFeatureCount();tr++)
	{
		fInfo->FI[tr].NegativeCount = 0;
		fInfo->FI[tr].PozitiveCount = 0;
	}
	fInfo->totalNegative = 0;
	fInfo->totalPozitive = 0;

	return true;
}
bool FeaturesStatistics::Init()
{
	UInt32		tr,rap;

	// creez obiectele:
	if ((notif = GML::Builder::CreateNotifier(Notifier.GetText()))==NULL)
		return false;
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
	if ((fData = new FeaturesThreadData[threadsCount])==NULL)
	{
		notif->Error("[%s] -> Unable to create %d FeaturesThreadData ",ObjectName,threadsCount);
		return false;
	}
	if (CreateFeaturesInfo(&All)==false)
		return false;
	// pornesc threadPool-ul
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
		if (CreateFeaturesInfo(&fData[tr])==false)
			return false;
	}
	// splituim
	rap = (con->GetRecordCount()/threadsCount)+1;

	// aloc memorie pentru indexi
	for (tr=0;tr<threadsCount;tr++)
	{
		if (tr+1==threadsCount)
			fData[tr].Range.Set(tr*rap,con->GetRecordCount());
		else
			fData[tr].Range.Set(tr*rap,(tr+1)*rap);
	}

	return true;
}
void FeaturesStatistics::OnRunThreadCommand(FeaturesThreadData &ftd,UInt32 command)
{
	UInt32	tr,gr,count;

	count = con->GetFeatureCount();
	// citesc datele asociate range-ului
	for (tr=ftd.Range.Start;(tr<ftd.Range.End) && (StopAlgorithm==false);tr++)
	{
		if (con->GetRecord(ftd.Record,tr)==false)
		{
			notif->Error("[%s] -> Unable to read record %d",ObjectName,tr);
			return;
		}
		if (ftd.Record.Label==1)
			ftd.totalPozitive++;
		else
			ftd.totalNegative++;
		for (gr=0;(gr<count) && (StopAlgorithm==false);gr++)
			if (ftd.Record.Features[gr]!=0)
			{
				if (ftd.Record.Label==1)
					ftd.FI[gr].PozitiveCount++;
				else
					ftd.FI[gr].NegativeCount++;
			}
	}
}
bool FeaturesStatistics::CreateHeaders(GML::Utils::GString &str)
{
	UInt32				tr;
	GML::Utils::GString	tmp;

	if (columnWidth==0)
	{
		if (str.Set("#|Pozitive|Negative|")==false)
			return false;
	} else {
		if (str.Set("    #|Pozitive|Negative|")==false)
			return false;
	}

	for (tr=0;tr<StatsData.Len();tr++)
	{
		if (tmp.Set(StatsData[tr].Name)==false)
			return false;
		while (tmp.Len()<columnWidth)
		{
			if (tmp.Add("                                                                                                     ")==false)
				return false;
		}
		if (columnWidth>0)
			tmp.Truncate(columnWidth);
		if (str.Add(&tmp)==false)
			return false;
		if (str.Add("|")==false)
			return false;
	}
	return true;
}
bool FeaturesStatistics::CreateRecordInfo(FeaturesInformations &finf,GML::Utils::GString &str)
{
	UInt32				tr;
	GML::Utils::GString	tmp;

	if (columnWidth==0)
	{
		if (str.SetFormated("%d|%d|%d|",finf.Index,(UInt32)finf.countPozitive,(UInt32)finf.countNegative)==false)
			return false;
	} else {
		if (str.SetFormated("%5d|%8d|%8d|",finf.Index,(UInt32)finf.countPozitive,(UInt32)finf.countNegative)==false)
			return false;
	}
	for (tr=0;tr<StatsData.Len();tr++)
	{
		if (tmp.SetFormated("%.4lf",finf.fnValue[tr])==false)
			return false;
		while (tmp.Len()<columnWidth)
		{
			if (tmp.Insert(" ",0)==false)
				return false;
		}
		if (columnWidth>0)
			tmp.Truncate(columnWidth);
		if (str.Add(&tmp)==false)
			return false;
		if (str.Add("|")==false)
			return false;
	}
	return true;
}

void FeaturesStatistics::PrintStats()
{
	UInt32					tr;
	GML::Utils::GString		str,line;

	if (CreateHeaders(str)==false)
	{
		notif->Error("[%s] -> Unable to create table headers !",ObjectName);
		return;
	}
	line.Set("");
	while (line.Len()<str.Len())
		line.AddChar('-');
	notif->Info(str.GetText());
	notif->Info(line.GetText());
	for (tr=0;tr<con->GetFeatureCount();tr++)
	{
		if (CreateRecordInfo(ComputedData[tr],str)==false)
		{
			notif->Error("[%s] -> Unable to create record line !",ObjectName);
			return;
		}
		notif->Info(str.GetText());
	}
	notif->Info(line.GetText());
}
void FeaturesStatistics::SaveToFile()
{
	GML::Utils::File	f;

	if (f.Create(ResultFile.GetText())==false)
	{
		notif->Error("[%s] -> Unable to create file : %s",ObjectName,ResultFile.GetText());
		return;
	}
	UInt32					tr;
	GML::Utils::GString		str,line;

	if (CreateHeaders(str)==false)
	{
		notif->Error("[%s] -> Unable to create table headers !",ObjectName);
		return;
	}
	line.Set("");
	while (line.Len()<str.Len())
		line.AddChar('-');
	str.Add("\n");
	line.Add("\n");
	if (f.Write(str.GetText(),str.Len())==false)
	{
		notif->Error("[%s] -> Unable to write to file : %s",ObjectName,ResultFile.GetText());
		return;
	}
	if (f.Write(line.GetText(),str.Len())==false)
	{
		notif->Error("[%s] -> Unable to write to file : %s",ObjectName,ResultFile.GetText());
		return;
	}

	for (tr=0;tr<con->GetFeatureCount();tr++)
	{
		if (CreateRecordInfo(ComputedData[tr],str)==false)
		{
			notif->Error("[%s] -> Unable to create record line !",ObjectName);
			return;
		}
		str.Add("\n");
		if (f.Write(str.GetText(),str.Len())==false)
		{
			notif->Error("[%s] -> Unable to write to file : %s",ObjectName,ResultFile.GetText());
			return;
		}		
	}
	if (f.Write(line.GetText(),str.Len())==false)
	{
		notif->Error("[%s] -> Unable to write to file : %s",ObjectName,ResultFile.GetText());
		return;
	}
	f.Close();
	notif->Info("[%s] -> %s table saved !",ObjectName,ResultFile.GetText());
}
void FeaturesStatistics::SaveFeatureWeightFile()
{
	GML::Utils::GTFVector<double>	list;
	GML::Utils::AttributeList		attr;
	UInt32							tr;

	// adaug elementele
	for (tr=0;tr<ComputedData.Len();tr++)
	{
		if (list.Push(ComputedData[tr].fnValue[saveFeatureWeightFile])==false)
		{
			notif->Error("[%s] -> Unable to add elements to weight vector in file : %s",ObjectName,FeaturesWeightFile.GetText());
			return;
		}
	}

	if (attr.AddAttribute("Weight",list.GetPtrToObject(0),GML::Utils::AttributeList::DOUBLE,list.Len())==false)
	{
		notif->Error("[%s] -> Unable to create AttributeList in file : %s",ObjectName,FeaturesWeightFile.GetText());
		return;
	}
	if (attr.AddString("Method",StatsData[saveFeatureWeightFile].Name)==false)
	{
		notif->Error("[%s] -> Unable to create AttributeList in file : %s",ObjectName,FeaturesWeightFile.GetText());
		return;
	}
	if (attr.AddDouble("MultiplyFactor",multiplyFactor)==false)
	{
		notif->Error("[%s] -> Unable to create AttributeList in file : %s",ObjectName,FeaturesWeightFile.GetText());
		return;
	}
	if (attr.Save(FeaturesWeightFile.GetText())==false)
	{
		notif->Error("[%s] -> Unable to save file : %s",ObjectName,FeaturesWeightFile.GetText());
		return;
	}
	notif->Info("[%s] -> File (%s) with '%s' features saved",ObjectName,FeaturesWeightFile.GetText(),StatsData[saveFeatureWeightFile].Name);
}
bool FeaturesStatistics::Compute()
{
	UInt32					tr,gr;
	FeaturesInformations	info;
	

	notif->Info("[%s] -> Computing statistics ...",ObjectName);
	// executie
	for (tr=0;tr<threadsCount;tr++)
		if (tpu[tr].Execute(1)==false)
		{
			notif->Error("Error on runnig thread #%d",tr);
			return false;
		}
	// asteptare
	for (tr=0;tr<threadsCount;tr++)
		if (tpu[tr].WaitToFinish()==false)
		{
			notif->Error("WaitToFinish failed on thread #%d",tr);
			return false;
		}
	// fac aditia
	for (tr=0;tr<threadsCount;tr++)
	{
		for (gr=0;gr<con->GetFeatureCount();gr++)
		{
			All.FI[gr].PozitiveCount+=fData[tr].FI[gr].PozitiveCount;
			All.FI[gr].NegativeCount+=fData[tr].FI[gr].NegativeCount;
		}
		All.totalNegative+=fData[tr].totalNegative;
		All.totalPozitive+=fData[tr].totalPozitive;
	}
	notif->Info("[%s] -> DataBase processed ok",ObjectName);
	// calculez si functiile
	for (tr=0;tr<con->GetFeatureCount();tr++)
	{
		info.Index = tr;
		info.totalPozitive = All.totalPozitive;
		info.totalNegative = All.totalNegative;
		info.countNegative = All.FI[tr].NegativeCount;
		info.countPozitive = All.FI[tr].PozitiveCount;
		if ((info.fnValue = new double[StatsData.Len()])==NULL)
		{
			notif->Error("[%s] Unable to allocate info.fnValue[%d] !",ObjectName,StatsData.Len());
			return false;
		}
		for (gr=0;gr<StatsData.Len();gr++)
			info.fnValue[gr] = StatsData[gr].fnCompute(&info) * multiplyFactor;

		if (ComputedData.PushByRef(info)==false)
		{
			notif->Error("[%s] Unable to add informations to vector !",ObjectName);
			return false;
		}
	}
	// verific daca am vreun weight de salvat
	if (saveFeatureWeightFile<StatsData.Len())
	{
		if (FeaturesWeightFile.Len()==0)
		{
			notif->Error("[%s] In order to save '%s' values for a feature you have to complete 'FeaturesWeightFile' property with the name of the file where the features will be saved",ObjectName,StatsData[saveFeatureWeightFile].Name);
		} else {
			SaveFeatureWeightFile();
		}
	}

	// sortez
	if (sortBy!=0xFFFF)
	{
		ComputedData.Sort(sortDirection==0,Compare_FeaturesInformations,&sortBy);
	}
	// printez
	if (notifyResults)
		PrintStats();
	// salvez
	if (ResultFile.Len()>0)
		SaveToFile();
	return true;
}
void FeaturesStatistics::OnExecute(char *command)
{
	StopAlgorithm = false;
	if (GML::Utils::GString::Equals(command,"compute",true))
	{
		Compute();
		return;
	}
	notif->Error("Unkwnown command: %s",command);
}