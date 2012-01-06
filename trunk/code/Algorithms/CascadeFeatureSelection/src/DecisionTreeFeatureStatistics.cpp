#include "DecisionTreeFeatureStatistics.h"


DecisionTreeFeatureStatistics::DecisionTreeFeatureStatistics()
{
	ObjectName 							= "DecisionTreeFeatureStatistics";
	callThreadComputeExtraDataFunction 	= true;
	
	LinkPropertyToUInt32("MaximumDepth",MaxDepth,0x0,"Maximum depth for the binary tree");
}
bool DecisionTreeFeatureStatistics::Init()
{
	if (GenericFeatureStatistics::Init()==false)
		return false;
		
	if (BTree.Create(256)==false)
	{
		notif->Error("[%s] -> Unable to create BTree !",ObjectName);
		return false;		
	}
	TreePathSize = 0;
	if (HashesForRecord.Create(con->GetRecordCount(),true)==false)
	{
		notif->Error("[%s] -> Unable to create HashesForRecord !",ObjectName);
		return false;		
	}
	MEMSET(HashesForRecord.GetPtrToObject(0), 0, sizeof(UInt64)*con->GetRecordCount());
	return true;
}
bool DecisionTreeFeatureStatistics::CreatePath(UInt32 index)
{
	UInt32	*bTree = BTree.GetPtrToObject(0);
	UInt32	remove;
	
	TreePathSize = 0;
	if (index==0)
		return true;
	while (index>0)
	{
		if (TreePathSize>=MAX_PATH_DEPTH)
		{
			notif->Error("[%s] -> Depth to hi (>%d) in BTree for index:%d",ObjectName,MAX_PATH_DEPTH,index);
			return false;
		}
		remove = (((index-1) & 1)<<31);
		index = (index-1)>>1;
		TreePath[TreePathSize++] = bTree[index] | remove;		
	}
	return true;
}
bool DecisionTreeFeatureStatistics::WriteData(GML::Utils::File& out)
{
	GML::Utils::GString		md5,temp,raw;
	UInt32					tr;
	GML::DB::RecordHash		rHash;
	double					label;
	
	if (temp.Create(0x10000)==false)
	{
		notif->Error("[%s] -> Unable to alloc memory for cache",ObjectName);
		return false;
	}
	temp.Set("");
	for (tr=0;tr<con->GetRecordCount();tr++)
	{
		if (con->GetRecordLabel(label,tr)==false)
		{
			notif->Error("[%s] -> Unable to read label for #%d",ObjectName,tr);
			return false;
		}
		if (con->GetRecordHash(rHash,tr)==false)
		{
			notif->Error("[%s] -> Unable to read record hash for #%d",ObjectName,tr);
			return false;
		}
		if (rHash.ToString(md5)==false)
		{
			notif->Error("[%s] -> Unable to convert record hash for #%d",ObjectName,tr);
			return false;
		}
		raw.Set(md5);
		raw.AddFormated("|%d|",(label==1.0?1:0));
		raw.AddFormatedEx("%{uint64,bin,L%%,F0,be}|0x%{uint64,Hex}\n", HashesForRecord[tr], TreePathSize-1,HashesForRecord[tr]);
		temp.Add(raw);
		if (temp.Len()>64000)
		{						
			if (out.Write(temp.GetText(),temp.Len())==false)
			{
				notif->Error("[%s] -> Unable to write data",ObjectName);
				return false;
			}
			temp.Truncate(0);
			temp.Set("");
		}
	}
	if (temp.Len()>0)
	{			
		if (out.Write(temp.GetText(),temp.Len())==false)
		{
			notif->Error("[%s] -> Unable to write data",ObjectName);
			return false;
		}
	}
	
	return true;
}
bool DecisionTreeFeatureStatistics::WritePaths(GML::Utils::File& out)
{
	UInt64					leafsCount,tr;
	GML::Utils::GString 	tmp,tmp2,toPrint,buff;
	UInt32					gr;
	
	if (buff.Create(0x10000)==false)
	{
		notif->Error("[%s] -> Unable to alloc memory for cache",ObjectName);
		return false;
	}
	buff.Set("");
	leafsCount = 1<<(TreePathSize-1);
	for (tr=0;tr<leafsCount;tr++)
	{
		toPrint.Set("");
		tmp2.Set("");
		if (CreatePath(BTree.Len()-tr-2)==false)
			return false;
		for (gr=0;gr<TreePathSize;gr++)
		{
			tmp.Set("");
			if (TreePath[gr] & 0x80000000)
			{
				tmp.Add("-");
				tmp2.Insert("1",0);
			} else {
				tmp.Add("+");
				tmp2.Insert("0",0);
			}
			tmp.AddFormated("%d",TreePath[gr] & 0x7FFFFFFF);
			tmp.Add("|");
			toPrint.Insert(tmp,0);
		}
		toPrint.Add("\n");
		tmp2.Add("|");
		toPrint.Insert(tmp2,0);
		buff.Add(toPrint);
		if (buff.Len()>64000)
		{						
			if (out.Write(buff.GetText(),buff.Len())==false)
			{
				notif->Error("[%s] -> Unable to write data",ObjectName);
				return false;
			}
			buff.Truncate(0);
			buff.Set("");
		}
	}
	if (buff.Len()>0)
	{			
		if (out.Write(buff.GetText(),buff.Len())==false)
		{
			notif->Error("[%s] -> Unable to write data",ObjectName);
			return false;
		}
	}
	return true;
}
bool DecisionTreeFeatureStatistics::WriteFlags(GML::Utils::File& out)
{
	GML::Utils::GString		feats,featName;
	UInt32					tr;
	UInt32					*bTree = BTree.GetPtrToObject(0);
	
	if (feats.Create(0x10000)==false)
	{
		notif->Error("[%s] -> Unable to alloc memory for cache",ObjectName);
		return false;
	}
	feats.Set("");
	for (tr=0;tr<con->GetFeatureCount();tr++)
	{
		if (con->GetFeatureName(featName, tr) == false)
		{
			notif->Error("[%s] -> Unable to get feature name for feature with index: %d",ObjectName, tr);
			return false;
		}		
		feats.AddFormated("%d|%s\n",tr,featName.GetText());
		if (feats.Len()>64000)
		{						
			if (out.Write(feats.GetText(),feats.Len())==false)
			{
				notif->Error("[%s] -> Unable to write data",ObjectName);
				return false;
			}
			feats.Truncate(0);
			feats.Set("");
		}
	}
	if (feats.Len()>0)
	{			
		if (out.Write(feats.GetText(),feats.Len())==false)
		{
			notif->Error("[%s] -> Unable to write data",ObjectName);
			return false;
		}
	}
	
	return true;
}
void DecisionTreeFeatureStatistics::OnCompute()
{
	UInt32				tr,gr;
	GML::Utils::File	out;	
	GML::Utils::GString tmp;
	
	if (out.Create(ResultFileName.GetText(),true)==false)
	{
		notif->Error("[%s] -> Unable to create: %s",ObjectName,ResultFileName.GetText());
		return;
	}
	tmp.Create(2048);
	BTree.DeleteAll();
	workingRecordsCount = con->GetRecordCount();

	while (TreePathSize<=MaxDepth)
	{
		// calculez counterele
	
		if (CreatePath(BTree.Len())==false)
			return;
		CreateWorkingList();
		// afisez working list;
		tmp.SetFormated("Working List (%d):",TreePathSize);
		for (tr=0;tr<TreePathSize;tr++)
		{
			if (TreePath[tr] & 0x80000000)
				tmp.Add("-");
			else
				tmp.Add("+");
			tmp.AddFormated("%d",TreePath[tr] & 0x7FFFFFFF);
			tmp.Add(",");
		}
		notif->Info("[%s] -> %s",ObjectName,tmp.GetText());
		
		ComputeScoresAndSort();
		if (FeatScores.Len()==0)
		{
			notif->Info("[%s] -> No more features left ... ending",ObjectName);
			break;
		}
		
		
		featToRemove = FeatScores[0].Index;
		BTree.PushByRef(FeatScores[0].Index);
		
		// afisez
		tmp.SetFormated("%d|Scor:%lf|Poz:%d|Neg:%d|TotalPoz:%d|TotalNeg:%d\n",
						featToRemove,
						FeatScores[0].Score,
						FeatCounters[featToRemove].CountPozitive,
						FeatCounters[featToRemove].CountNegative,
						FeatCounters[featToRemove].CountTotalPozitive,
						FeatCounters[featToRemove].CountTotalNegative);
		
		//out.Write(tmp.GetText(),tmp.Len());
	}	
	if (WriteData(out) == false)
	{
		notif->Error("[%s] -> Unable to write hashes to file: %s",ObjectName,ResultFileName.GetText());
		return;
	}
	tmp.Set("=========================================================\n");
	tmp.Add("==========================FLAGS==========================\n");
	tmp.Add("=========================================================\n");
	out.Write(tmp.GetText(),tmp.Len());
	if (WriteFlags(out) == false)
	{
		notif->Error("[%s] -> Unable to write leafs to file: %s",ObjectName,ResultFileName.GetText());
		return;		
	}
	tmp.Set("=========================================================\n");
	tmp.Add("==========================LEAFS==========================\n");
	tmp.Add("=========================================================\n");
	out.Write(tmp.GetText(),tmp.Len());
	if (WritePaths(out) == false)
	{
		notif->Error("[%s] -> Unable to write leafs to file: %s",ObjectName,ResultFileName.GetText());
		return;		
	}
	tmp.Set("=========================================================\n");
	out.Write(tmp.GetText(),tmp.Len());
	out.Close();
}

bool DecisionTreeFeatureStatistics::OnThreadComputeExtraData(UInt32 recordIndex,GML::Algorithm::MLThreadData &thData)
{
	if ((TreePathSize) && (TreePath[0] & 0x80000000))
	{
		HashesForRecord[recordIndex] |= (1<<(TreePathSize-1));
	}
	return true;
}
