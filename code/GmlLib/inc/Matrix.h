#ifndef __MATRIX_H
#define __MATRIX_H

#include "Vector.h"
#include "Compat.h"
#define DIM_MAX 1000

namespace GML
{
	namespace Utils
	{
	
		template <class T> class Matrix
		{
		private:
			//the element "holder"
			GML::Utils::Vector			v;
			//and the sizes of matrix
			unsigned int	nr_lines,
							nr_colls;
			//current allocated space
			unsigned int	max_lines,
							max_colls;
		public:
			//cosntructor - empty matrix
			Matrix();
			Matrix(Matrix<T>& m);
			~Matrix();
			bool Create(unsigned int nrLines=10U,unsigned int nrColl=10U);
			bool SetElement(unsigned int line,unsigned int coll,T element);
			T GetElement(unsigned int line,unsigned int coll);
			unsigned int GetNrColl();
			unsigned int GetNrLines();
			unsigned int GetElementSize();
			unsigned int GetSize();
			Matrix<T> operator+(Matrix<T>& matrixRight);
			Matrix<T> operator*(Matrix<T>& matrixRight);
			bool Resize(unsigned int nrLines = 1U,unsigned int nrColls = 1U);
			bool ResetLine(unsigned int line);
			bool ResetCollumn(unsigned int coll);
			void Reset();
#endif

