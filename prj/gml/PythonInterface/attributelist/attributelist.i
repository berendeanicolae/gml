%module attributelist

%include std_string.i
%include std_pair.i
%include std_map.i

%{
#define SWIG_FILE_WITH_INIT
#include "..\..\..\code\GmlLib\inc\AttributeList.h"
#include "..\..\..\code\GmlLib\inc\Compat.h"
%}


%template() std::pair<swig::SwigPtr_PyObject, swig::SwigPtr_PyObject>;
%template(pymap) std::map<swig::SwigPtr_PyObject, swig::SwigPtr_PyObject>;


namespace GML
{
	namespace Utils
	{
		struct Attribute
		{
			char*			Name;
			char*			Description;
			unsigned char*	Data;
			unsigned int	DataSize;
			unsigned int	AttributeType;
			unsigned int	ElementsCount;

			bool operator < (Attribute &a1);
			bool operator > (Attribute &a1);
		};

		class AttributeList
		{
			GML::Utils::GTVector<GML::Utils::Attribute>	list;
			%feature("autodoc", "1");
			bool			FromString(GML::Utils::GString &text);
		public:
			enum 
			{
				BOOLEAN = 0,
				INT8,INT16,INT32,INT64,
				UINT8,UINT16,UINT32,UINT64,
				FLOAT,DOUBLE,
				STRING,
				ATTRIBUTES_COUNT		
			};		
		public:
			AttributeList(void);
			~AttributeList(void);
		    %feature("autodoc", "1");
			bool			AddAttribute(char* Name,void *Data,unsigned int AttributeType,unsigned int ElementsCount=1,char *Description=NULL);
			%feature("autodoc", "1");
			bool			AddString(char *Name, char *Text, char *Description = NULL);
			%feature("autodoc", "1");
			bool			AddBool(char *Name, bool value,char *Description = NULL);
			%feature("autodoc", "1");
			bool			AddDouble(char *Name, double value, char *Description = NULL);
			//%feature("autodoc", "1");
			//bool			AddUInt32(char *Name, UInt32 value, char *Description = NULL);
			%feature("autodoc", "1");
			bool			AddInt32(char *Name, int value, char *Description);
			%feature("autodoc", "1");
			bool			Update(char *Name,void *Data,UInt32 DataSize);
			%feature("autodoc", "1");
			
			%feature("autodoc", "1");
			void			Clear();
			%feature("autodoc", "1");
			Attribute*		Get(unsigned int index);
			%feature("autodoc", "1");
			Attribute*		Get(char* Name);
			%feature("autodoc", "1");
			unsigned int	GetCount();
			%feature("autodoc", "1");
			bool			Save(char *fileName);
			%feature("autodoc", "1");
			bool			Load(char *fileName);
			%feature("autodoc", "1");
			bool			Create(char *text,char separator=';');
						
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

					GML::Utils::AttributeList * al = self;
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
							bool x = PyObject_IsTrue(it->second)==1? true: false; 	
							if (!al->AddBool(str1, x)) 
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
					return true;
				}				
			}											
											
		};
	}
}







