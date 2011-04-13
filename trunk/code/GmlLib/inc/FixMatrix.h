#ifndef __FIXMATRIX_H__
#define __FIXMATRIX_H__

#include "Compat.h"

namespace GML{
	namespace Utils{
			template <class T>
		class EXPORT FixMatrix{
		private:
			T* data;
			unsigned int	lines,
							collumns;
		public:
			FixMatrix(unsigned int lines,unsigned int collumns){
				if((data = new T[lines*collumns*sizeof(T)] )== NULL){
					data = NULL;
					this->lines = this->collumns = 0;
				}else{
					this->lines = lines;
					this->collumns = collumns;
				}
			}
			FixMatrix(){data = NULL;lines=collumns=0;}
			~FixMatrix(){
				Free();
			}

			bool Create(unsigned int lines,unsigned int collumns){
				T* temp;
				if(this->lines == lines && this->collumns == collumns){
					return true;
				}
				if((temp = new T[lines*collumns*sizeof(T)] )== NULL){
					return false;
				}
				data = temp;
				this->lines		= lines;
				this->collumns  = collumns;
				return true;
			}
			//It works directly on the result object so you have to take care
			bool Multiply(FixMatrix<T>& m,FixMatrix<T>& result){
				if(this->collumns != m.lines){
					//is has 0 size
					return false;
				}

				unsigned int r1 = this->lines;
				unsigned int c2 = m.collumns;
				
				if(result.Create(this->lines,m.collumns) == false){
					return false;
				}

				for(unsigned int i=0;i<this->lines;i++){
					for(unsigned int j=0; j<m.collumns; j++){
						result.data[i * result.collumns + j] = 0;
						for(unsigned int k=0; k < this->collumns; k++){
							result.data[i*result.collumns+j] += (this->data[i*this->collumns + k] * m.data[k*m.collumns + j]);
						}
					}
				}
				return true;
			}
			//It works directly on the result object so you have to take care
			bool Add(FixMatrix<T>&m, FixMatrix<T>& result){
				unsigned int i,j;
				if(this->collumns!= m.collumns|| this->lines != m.lines){
					//is has 0 size
					return false;
				}
				if(result.Create(this->lines,this->collumns) == false){
					return false;
				}
				for(i=0;i<this->lines;i++)
					for(j=0 ; j<this->collumns; j++)
						m.data[i*m.collumns+ j] = this->data[i*this->collumns + j] + m.data[i*m.collumns+ j];
				return true;
			}

			T* GetValue(unsigned int line,unsigned int collumn){
				if(line >= this->lines || collumn >= this->collumns){
					return NULL;
				}
				return &this->data[line*this->collumns + collumn];
			}

			bool setValue(unsigned int line,unsigned int collumn, T val){
				if(line >= this->lines || collumn >= this->collumns){
					return false;
				}
				this->data[line*this->collumns + collumn] = val;
				return true;
			}

			void Free(){
				if (data!=NULL) delete data;
				lines = collumns = 0;	
				data=NULL;
			}
		};
	}
}
#endif