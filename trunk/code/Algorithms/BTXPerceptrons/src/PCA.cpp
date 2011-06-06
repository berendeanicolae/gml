#include "PCA.h"

PCA::PCA(){
	
	ObjectName = "PCA";
	LinkPropertyToUInt32("PcaNr",pca_nr,0,"The number of principal components");
	LinkPropertyToDouble("Thereshold",thereshold,0.001,"Convergence check ....");
	//SetPropertyMetaData("Command","!!LIST:None=0,Test!!");
}

bool PCA::Init(){
	unsigned int idx_r,idx_f;
	
	if (InitConnections()==false)
		return false;
	rc = con->GetRecordCount();
	fc = con->GetFeatureCount();

	if(t.Create(rc) == false){
		notif->Error("[%s] Nu s-a putut creea vectorul de scoruri - t",ObjectName);
		return false;
	}
	if(p.Create(fc) == false){
		notif->Error("[%s] Nu s-a putut creea vectorul de loadings - p",ObjectName);
		return false;
	}
	if (con->CreateMlRecord(ob)==false)
	{
		notif->Error("[%s] -> Unable to create ob-urile",ObjectName);
		return false;
	}
	matrix.Create(rc,fc);
	for(idx_r=0;idx_r<rc;idx_r++){
		if( con->GetRecord(ob,idx_r) == false){
			notif->Error("[%s] Nu s-a putut initia vectorul de scoruri (1)",ObjectName);
			return false;
		}
		for(idx_f=0;idx_f<fc;idx_f++){
			matrix.setValue(idx_r,idx_f,ob.Features[idx_f]);
		}
		t.Insert(ob.Features[0],idx_r);
	}
	if(filePath.SetFormated("PCA.txt") == false){
		notif->Error("[%s] Can't set formated PCA.txt",ObjectName);
		return false;
	}
	notif->Info("[%s] Initializarea a avut loc cu succes.",ObjectName);
	return true;
}

void PCA::OnExecute(){
	unsigned int	idx_r,idx_f,idx_pca;
	double			ttt,
					ttt_old,
					ppt;

	GML::Utils::GString s;
	GML::Utils::AttributeList a;
	
	GML::Utils::GTFVector<double> center;

	notif->Info("[%s] Centram matricea",ObjectName);
	center.Create(fc);
	for(idx_f=0;idx_f<fc;idx_f++){
		center.Insert(0,idx_f);
	}

	for(idx_r=0;idx_r<rc;idx_r++){
		for(idx_f=0;idx_f<fc;idx_f++){
			center[idx_f] += *(matrix.GetValue(idx_r,idx_f));
		}
	}
	for(idx_r=0;idx_r<rc;idx_r++){
		center[idx_f] /= rc;
	}

	for(idx_r=0;idx_r<rc;idx_r++){
		for(idx_f=0;idx_f<fc;idx_f++){
			(*matrix.GetValue(idx_r,idx_f)) -= center[idx_f];
		}
	}
	for(idx_pca=0;idx_pca<this->pca_nr;idx_pca++)
	{
		ttt = 0;
		for(idx_f=0;idx_f<rc;idx_f++)
			ttt += t[idx_f]*t[idx_f];

		notif->Info("[%s] Primul pas a inceput: Proiectarea lui X pe t ",ObjectName);
		if(ttt<this->thereshold)
		{
			notif->Error("[%s] ttt is 0. Aborting PCA",ObjectName);
			return;
		}
		
		do{
			//backup the eigenvalue
			ttt_old = ttt;
			for(int i=0;i<fc;i++)
				p.Insert(0,i);
			for(idx_r=0;idx_r<rc;idx_r++){
				for(idx_f=0;idx_f<fc;idx_f++){
					p[idx_f] += t[idx_r]*(*(matrix.GetValue(idx_r,idx_f)));
				}
			}//--verified
			ppt = 0;
			for(idx_f=0;idx_f<fc;idx_f++){
				p[idx_f] /= ttt;
				ppt += p[idx_f]*p[idx_f];
			}

			ppt = sqrt(ppt);
			notif->Info("[%s] Al doilea pas a inceput: Normalizarea lui p",ObjectName);
			for(idx_f=0;idx_f<fc;idx_f++){
				p[idx_f] /= ppt;
			}//--verified
			notif->Info("[%s] Al trei-lea pas a inceput: Proiectia lui X pe p",ObjectName);
			ppt = 0;
			for(idx_f=0;idx_f<fc;idx_f++){
				ppt += p[idx_f]*p[idx_f];
			}
			if(ppt<this->thereshold){
				notif->Error("[%s] ppt is 0. Aborting PCA",ObjectName);
				return;
			}
			for(idx_r=0;idx_r<rc;idx_r++){
				t[idx_r] = 0;
				
				for(idx_f=0;idx_f<fc;idx_f++){
					t[idx_r] += (*matrix.GetValue(idx_r,idx_f))*p[idx_f];
				}
				t[idx_r] /= ppt;
			}
			notif->Info("[%s] Al patru lea pas a inceput: verificarea convergentei.",ObjectName);
			//the new eigenvalue
			ttt = 0;
			for(idx_f=0;idx_f<rc;idx_f++)
				ttt += t[idx_f]*t[idx_f];
		}
		while(ttt*(1-thereshold)>ttt_old);
		notif->Info("Am gasit a %d-a cea mai mare componenta principala. eigenvalue: %fl",(idx_pca+1),ttt);
		notif->Info("[%s] Al cinci-lea pas: eliminam componenta principala din date",ObjectName);
		for(idx_r=0;idx_r<rc;idx_r++){
			
			for(idx_f=0;idx_f<fc;idx_f++){
				*matrix.GetValue(idx_r,idx_f) -= t[idx_r]*p[idx_f];
			}
		}
		
		if(s.SetFormated("p%d",idx_pca) == false){
			notif->Error("[%s] Nu se poate salva PCA loadings.",ObjectName);
		}else{
			a.AddAttribute(s.GetText(), (void*)p.GetVector(), GML::Utils::AttributeList::DOUBLE, p.Len());
		}
		if(s.SetFormated("t%d",idx_pca) == false){
			notif->Error("[%s] Nu se poate salva PCA scores.",ObjectName);
		}else{
			a.AddAttribute(s.GetText(), (void*)t.GetVector(), GML::Utils::AttributeList::DOUBLE, t.Len());
		}
		
		a.Save(filePath.GetText());
	}
}

bool PCA::OnInitThreadData(GML::Algorithm::MLThreadData &thData){
	return true;
}
void PCA::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand){

}