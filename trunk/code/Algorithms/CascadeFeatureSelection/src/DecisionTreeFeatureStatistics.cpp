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
		// afisez working list
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
		
		out.Write(tmp.GetText(),tmp.Len());
	}	
	out.Close();
	GML::Utils::GString	temp, 	hash;
	GML::DB::RecordHash			rHash;
	for (tr=0;tr<con->GetRecordCount();tr++)
	{
		if (con->GetRecordHash(rHash,tr)==false)
		{
			notif->Error("[%s] -> Unable to read record hash for #%d",ObjectName,tr);
			//return false;
		}
		if (rHash.ToString(hash)==false)
		{
			notif->Error("[%s] -> Unable to convert record hash for #%d",ObjectName,tr);
			//return false;
		}
		temp.Set("");
		temp.AddFormatedEx("%{uint64,bin}", HashesForRecord[tr]);
		notif->Info("[%s] -> %s|%s",ObjectName, hash.GetText(), temp.GetText());
	}
	
	
}

bool DecisionTreeFeatureStatistics::OnThreadComputeExtraData(UInt32 recordIndex,GML::Algorithm::MLThreadData &thData)
{
	if ((TreePathSize) && (TreePath[0] & 0x80000000))
	{
		HashesForRecord[recordIndex] |= (1<<(TreePathSize-1));
	}
	/*
	if ((TreePathSize) && (thData.Record.Features[TreePath[0] & 0x7FFFFFFF] != 0))
	{
		UInt64 tmp;
		tmp = 1;
		tmp = tmp<<(64-TreePathSize);
		HashesForRecord[recordIndex] = HashesForRecord[recordIndex] | tmp;
	}
	/*
	if ((TreePathSize) && (thData.Record.Features[TreePath[0] & 0x7FFFFFFF] == 0) && (TreePath[0] & 0x80000000))
	{
		UInt64 tmp;
		tmp = 1;
		tmp = tmp<<(64-TreePathSize);
		HashesForRecord[recordIndex] = HashesForRecord[recordIndex] | tmp;
	}
	*/
	return true;
}
