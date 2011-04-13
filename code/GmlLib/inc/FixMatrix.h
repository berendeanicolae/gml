
namespace GML{
	namespace Utils{
		template <class T>
		class  FixMatrix{
			T* data;
			unsigned int	lines,
							collumns;
			FixMatrix(unsigned int lines,unsigned int collumns){
				if((data = new T[lines*collumns*sizeof(T)] )== NULL){
					data = NULL;
					lines = collumns = 0;
				}
			}
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
			bool Multiply(FixMatrix& m,FixMatrix& result){
				if(this->nr_colls != m.nr_lines){
					//is has 0 size
					return false;
				}

				unsigned int r1 = this->lines;
				unsigned int c2 = m.collumns;
				
				if(result.Create(this->lines,m.collumns) == false){
					return false;
				}

				for(unsigned int i=0;i<this->lines;i++){
					for(unsigned int j=0; j<this->collumns; j++){
						result.data[i * this->collumns + j] = 0;
						for(unsigned int k=0; k < this->collumns; k++){
							result.data[i*this->collumns+j] += (this->data[i*this->collumns + k] * m.data[k*this->collumns + j]);
						}
					}
				}
				return true;
			}
			bool Add(FixMatrix&m, FixMatrix& result){
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