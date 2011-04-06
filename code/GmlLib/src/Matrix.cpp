#include "Matrix.h"

template <class T> Matrix<T>::Matrix(void){
			nr_lines  = 0;
			nr_colls  = 0;
			//current allocated space
			max_lines = DIM_MAX;
			max_colls = DIM_MAX;
}

template <class T> Matrix<T>::Matrix(Matrix<T>& m){
			this->max_colls = m.max_colls;
			this->max_lines = m.max_lines;
			this->nr_colls = m.nr_colls;
			this->nr_lines = m.nr_lines;
			this->v = m.v;
}
template <class T> Matrix<T>::~Matrix(){
			nr_lines = 0;
			nr_colls = 0;
			max_lines = 0;
			max_colls = 0;
			v.Free();
}

//the way to alloc some space
template <class T> bool Matrix<T>::Create(unsigned int nrLines=10U,unsigned int nrColl=10U){
			//setting the allocated space of the matrix as a multiply of DIM_MAX
			if(nrColl > this->max_colls){
				this->max_colls = DIM_MAX*((unsigned int)nrColl/DIM_MAX + 1);
			}
			if(nrLines > this->max_lines){
				this->max_lines = DIM_MAX*((unsigned int)nrLines/DIM_MAX + 1);
			}
			//Allocate the space
			if(v.Create(max_lines * max_colls,sizeof(T)) == true){
				this->nr_colls = nrColl;
				this->nr_lines = nrLines;
				return true;
			}
			//if failure in constructing the vector
			this->max_lines = 0;
			this->max_colls = 0;
			this->nr_lines = 0;
			this->nr_colls = 0;
			return false;
}
			
// set a certain element. returns false if error
template <class T> bool Matrix<T>::SetElement(unsigned int line,unsigned int coll,T element){
			if(line >= this->max_lines || coll >= this->max_colls){
				return false;
			}
			((T*)v.GetVector())[line*this->max_colls + coll] = element;
			return true;
}
//gets a certain element
template <class T> T Matrix<T>::GetElement(unsigned int line,unsigned int coll){
			if(line >= this->max_colls || coll >= this->max_colls){
				return 0;
			}
			return ((T*)v.GetVector())[line*this->max_colls + coll]
}
		
template <class T> unsigned int Matrix<T>:: GetNrColl(){
			return this->nr_colls;
}
template <class T> unsigned int Matrix<T>:: GetNrLines(){
			return this->nr_lines;
}
template <class T> unsigned int Matrix<T>:: GetElementSize(){
			return this->v.GetElementSize();
}
template <class T> unsigned int Matrix<T>::GetSize(){
			return this->max_colls*max_lines;
}


//Add operator. 
template <class T> Matrix<T> Matrix<T>::operator+(Matrix<T>& matrixRight){
	Matrix<T> m;
	unsigned int i,j;
	if(this->GetSize() == 0|| this->GetElementSize() != matrixRight.GetElementSize() || this->nr_colls != matrixRight.nr_colls || this->nr_lines != matrixRight.nr_lines){
		//is has 0 size
		v.Free();
		return m;
	}
	m.Create(this->nr_lines,this->nr_colls);
	for(i=0;i<this->nr_lines;i++)
		for(j=0 ; j<this->nr_colls; j++)
			((T*)m.v.GetVector())[i*this->max_colls + j] = ((T*)this->v.GetVector())[i*this->max_colls + j] + ((T*)matrixRight.v.GetVector())[i*this->max_colls + j];
	for(i =this->nr_lines;i<this->max_lines;i++)
		m.ResetLine(i);
	for(i =this->nr_colls;i<this->max_colls;i++)
		m.ResetCollumn(i);
	return m;
}

//Multiply operator
template <class T> Matrix<T> Matrix<T>::operator*(Matrix<T>& matrixRight){
	Matrix<T> m;
	if(this->nr_colls != matrixRight.nr_lines || this->GetElementSize() != matrixRight.GetElementSize() || this->GetSize() == 0 || matrixRight.GetSize() == 0){
		//is has 0 size
		return m;
	}

	unsigned int r1 = this->nr_lines;
	unsigned int c2 = matrixRight.nr_colls;
	m.Create(r1,c2);
	T *v1 = (T*)this->v.GetVector(), *v2 = (T*)matrixRight.v.GetVector(), *v = (T*)m.v.GetVector();
	for(unsigned int i=0;i<r1;i++){
		for(unsigned int j=0; j<c2; j++){
			v[i * this->max_colls + j] = 0;
			for(unsigned int k=0; k<r1; k++){
				v[i*this->max_colls+j] += (v1[i*this->max_colls + k] * v2[k*this->max_colls + j]);
			}
		}
	}
	return m;
}

//Grow the matrix
template <class T> bool Matrix<T>::Resize(unsigned int nrLines = 1U,unsigned int nrColls = 1U){
	unsigned int tempC = this->max_colls;
	unsigned int tempL = this->max_lines;
	bool realloc = false;
	unsigned int i,j;

	//if there must be more than max lines already allocated space
	if(this->nr_lines + nrLines > this->max_lines){
		tempL = DIM_MAX*((unsigned int)(nrLines + this->nr_lines)/DIM_MAX + 1);
		realloc = true;
	}
	//if there must be more than max colls already allocated space
	if(this->nr_colls + nrColls > this->max_colls){
		tempC = DIM_MAX*((unsigned int)(this->nr_colls + nrColls)/DIM_MAX + 1);
		realloc = true;
	}
	//we resize the support vector
	if(realloc){
		if(this->v.Resize(tempC*tempL) ==false){
			return false;
		}
		//update the allocated dimensions to the real ones
		T* v = (T*)this->v.GetVector();

		//Reorder the data inside the vector
		for(i = this->nr_lines-1;i>0;i--){
			for(j = this->nr_colls-1;j!=0xffffffff;j--){
				v[i*tempC + j] = v[i*this->max_colls + j];
			}
		}

		this->max_lines = tempL;
		this->max_colls = tempC;

		//we set to zero the lines allocated, but not yet popullated
		for(i = this->nr_lines;i<this->max_lines;i++)
			ResetLine(i);
		for(i = this->nr_colls;i<this->max_colls;i++)
			ResetCollumn(i);
	}
				
	//and after that sets the correct number of colls/lines
	this->nr_colls += nrColls;
	this->nr_lines += nrLines;
				
	return true;
}
//sets to zero a certain line
template <class T> bool Matrix<T>::ResetLine(unsigned int line){
	if(line >= this->max_lines){
		return false;
	}
				
	for(unsigned int coll=0;coll<this->max_colls;coll++){
		this->SetElement(line,coll,0);
	}
	return true;
}
//sets to zero a certain column
template <class T> bool Matrix<T>::ResetCollumn(unsigned int coll){
	if(coll >= this->max_colls){
		return false;
	}
				
	for(unsigned int line=0;line<this->max_lines;line++){
		this->SetElement(line,coll,0);
	}
	return true;
}

//resets the matrix
template <class T> void Matrix<T>::Reset(){
	for(unsigned int i=0;i<this->max_lines;i++){
		for(unsigned int j=0; j<this->max_colls; j++){		
			((T*)v.GetVector())[i * this->max_colls + j] = 0;
		}
	}
}