%module ialgorithm
%include std_string.i
%include std_pair.i
%include std_map.i


%{
#define SWIG_FILE_WITH_INIT
#include "..\..\..\code\GmlLib\inc\Compat.h";
#include "..\..\..\code\GmlLib\inc\IAlgorithm.h";
#include "..\..\..\code\GmlLib\inc\AttributeList.h"
%}


%template() std::pair<swig::SwigPtr_PyObject, swig::SwigPtr_PyObject>;
%template(pymap) std::map<swig::SwigPtr_PyObject, swig::SwigPtr_PyObject>;


namespace GML
{
	namespace Algorithm
	{
		class IAlgorithm
		{
		public:

			%feature("autodoc", "1");
			IAlgorithm();

			%feature("autodoc", "1");
			bool			SetProperty(GML::Utils::AttributeList &config);

			%feature("autodoc", "1");
			bool			SetProperty(char *config);

			%feature("autodoc", "1");
			bool			GetProperty(GML::Utils::AttributeList &config);
									
			%feature("autodoc", "1");
			virtual bool	Init() = 0;

			%feature("autodoc", "1");
			void	OnExecute(char *command);
			
			%feature("autodoc", "1");			
			bool	Execute(char *command);
			
			%feature("autodoc", "1");
			bool	Wait();

			%feature("autodoc", "1");
			bool	Wait(unsigned int nrMiliseconds);

			%extend 
			{								
				bool Set(const std::map<swig::SwigPtr_PyObject,swig::SwigPtr_PyObject>& v) 
				{
					std::string key;
					std::map<swig::SwigPtr_PyObject,swig::SwigPtr_PyObject>::const_iterator it, end;
					end = v.end();
					char *str;
					char str1[1024], str2[1024];
					size_t psize;
					int alloc;
					int nr = 0;
					end = v.end();					

					GML::Utils::AttributeList * al = new GML::Utils::AttributeList();
					if (al == NULL) return false;
					
					for (it = v.begin(); it != end; ++it)
					{						
						// check first parameter
						if (SWIG_AsCharPtrAndSize(it->first, &str, &psize, &alloc)==SWIG_OK)
						{					
							strncpy(str1, str, 1024);
						} else 
						{
							delete al;
							return false;
						}
						
						// check second parameter
						if (PyBool_Check(it->second))
						{
							//printf("second is a bool: %d\n", PyObject_IsTrue(it->second));	
							if (!al->AddBool(str1, (bool)PyObject_IsTrue(it->second))) 
							{
								delete al;
								return false;
							}
						} else

						if (PyFloat_Check(it->second)) 
						{
							//printf ("second is a float: %.03f\n", PyFloat_AsDouble(it->second));
							if (!al->AddDouble(str1, PyFloat_AsDouble(it->second))) 
							{
								delete al;
								return false;
							}
						} else

						if (PyInt_Check(it->second))
						{
							//printf("second is a int: %d\n", PyInt_AsLong(it->second));
							if (!al->AddUInt32(str1, PyInt_AsLong(it->second))) 
							{
								delete al;
								return false;
							}
						} else
						
						if (SWIG_AsCharPtrAndSize(it->second, &str, &psize, &alloc)==SWIG_OK)
						{
							//printf ("second is a string: \"%s\"\n", str);
							strncpy(str2, str, 1024);
							if (!al->AddString(str1, str2)) 
							{
								delete al;
								return false;
							}
						} 
					}
					bool x = self->SetProperty(*al);
					delete al;
					return x;
				}				
			}											

		};
	}
}







