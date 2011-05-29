import os,time,random

#Fill with the path for GML Package
GML_PATH = r"E:\lucru\GML\gml\prj\gml\Release"
GML_REV = 100

def __GetVarType(var):
	s = str(type(var))
	s = s.split("'",1)[1].rsplit("'",1)[0]
	return s

def __GenerateRandomFileName():
	name = "GML_TEMPLATE_"+str(hex(os.getpid()))+"_"+str(hex(int(time.time())))+"_"+str(hex(int(random.random()*100000000)))+".ini"
	return name.replace("0x","")


def __DictToString(dictionar):
	s = ""
	obj_name = ""
	for k in dictionar:
		if __GetVarType(k) != 'str':
			print("[Error] Key should be a string value "+str(k))
			return None
		if k.lower()=="type":
			obj_name=str(dictionar[k])
			continue
		s += (k+"=")
		v = dictionar[k]
		tip = __GetVarType(v)
		if tip == 'str':
			s+=(v+";")
		elif tip == 'int':
			s+=(str(v)+";")
		elif tip == 'float':
			s+=(str(v)+";")
		elif tip == 'bool':
			s+=(str(v)+";")
		else:
			print("[Error] Unknwon type for "+k+" => "+tip)
			return None
	if len(obj_name)==0:
		print("[Error] Missing 'type' property for "+str(dictionar))
		return NULL
	return obj_name+"{"+s+"}"
def __ListOfDictToString(lista):
	s = ""
	for item in lista:
		if __GetVarType(item) != 'dict':
			print("[Error] List should contains only dictionaries "+str(lista))
			return None
		r = __DictToString(item)
		if r==None:
			return None
		s += r+"=>"
	if len(s)<2:
		print("[Error] Invalid list: "+str(lista)+" => "+s)
		return None
	s = s[:-2]
	return s	
def __ListOfStringsToString(lista):
	s = ""
	for item in lista:
		if __GetVarType(item) != 'str':
			print("[Error] List should contains only strings "+str(lista))
			return None
		s += item+";"
	if len(s)==0:
		print("[Error] Invalid list: "+str(lista)+" => "+s)
		return None
	s = s[:-1]
	return s	
def __ListOfNumbersToString(lista):
	s = ""
	vname = __GetVarType(lista[0])
	if (vname=='bool'):
		s = "(BOOL)["
	elif (vname=='float'):
		s = "(DOUBLE)["
	elif (vname=='int'):
		s = "(INT32)["
	else:
		s = "["
	for item in lista:
		s+=str(item)+","
	if len(s)==0:
		print("[Error] Invalid list: "+str(lista)+" => "+s)
		return None
	s = s[:-1]+"]"
	return s	
def __ListToString(lista):
	if len(lista)==0:
		return "\"\""
	typeName = __GetVarType(lista[0])
	for item in lista:
		vname = __GetVarType(item)
		if (vname!=typeName):
			print("[Error] Invalid element in list ("+str(item)+") of type "+vname+" -> should be of type : "+typeName)
			return None
	if typeName == 'dict':
		return "\""+__ListOfDictToString(lista)+"\""
	elif typeName == 'str':
		return "\""+__ListOfStringsToString(lista)+"\""
	elif typeName == 'int':
		return __ListOfNumbersToString(lista)
	elif typeName == 'float':
		return __ListOfNumbersToString(lista)
	elif typeName == 'bool':
		return __ListOfNumbersToString(lista)
	else:
		print("[Error] Lists should contain dict elements or string elements only")
		return None	
def CreateTemplateFile(dictionar,fname,checkForAlgorithm = False):
	s = ""
	alg = False
	cmd = False
	for k in dictionar:
		if __GetVarType(k) != 'str':
			print("[Error] Key should be a string value "+str(k))
			return False
		if k.lower()=='algorithmname':
			alg = True
		if k.lower()=='command':
			cmd = True
		s += (k+" = ")
		v = dictionar[k]
		tip = __GetVarType(v)
		if tip == 'str':
			s+=("'"+v+"'\n")
		elif tip == 'int':
			s+=(str(v)+"\n")
		elif tip == 'float':
			s+=(str(v)+"\n")
		elif tip == 'bool':
			s+=(str(v)+"\n")
		elif tip == 'dict':
			r = __DictToString(v)
			if r==None:
				return False
			s+="\""+r+"\"\n"
		elif tip == 'list':
			r = __ListToString(v)
			if r==None:
				return False
			s+=r+"\n"
		else:
			print("[Error] Unknwon type for "+k+" => "+tip)
			return False
	if checkForAlgorithm:
		if alg==False:
			print("[Error] Missing 'Algorithm' property")
			return False
		if cmd==False:
			print("[Error] Missing 'Command' property")
			return False
	
	try:
		f = open(fname,"w")
		f.write(s)
		f.close()
		return True
	except:
		print("[Error] Unable to create/write: "+fname)
	
	return False
			
def Run(dictionar):
	fname = __GenerateRandomFileName()
	if __GetVarType(dictionar)=="instance":
		dictionar = dictionar._attr;
	if CreateTemplateFile(dictionar,fname,True)==False:
		return False
	os.system("\""+os.path.join(GML_PATH,"gml.exe")+"\" run \""+fname+"\"")
	try:
		os.unlink(fname)
	except:
		pass
	return True		
class AlgorithmResult:
	def __init__(self):
		self.Iteration = 0
		self.tp = 0
		self.tn = 0
		self.fp = 0
		self.fn = 0
		self.sp = 0.0
		self.se = 0.0
		self.acc = 0.0
		self.med = 0.0
		self.precision = 0.0
		self.FMeasure = 0.0
		self.fallout = 0.0	
	def Clear(self):
		self.Iteration = 0
		self.tp = 0
		self.tn = 0
		self.fp = 0
		self.fn = 0
		self.sp = 0.0
		self.se = 0.0
		self.acc = 0.0
		self.med = 0.0
		self.precision = 0.0
		self.FMeasure = 0.0
		self.fallout = 0.0
	def Update(self,classType,corectellyClasified,updateValue=1):
		if corectellyClasified:
			if classType:
				self.tp+=updateValue;
			else:
				self.tn+=updateValue;
		else:
			if classType:
				self.fn+=updateValue;
			else:
				self.fp+=updateValue;
	def Compute(self):
		sum1 = float(self.tp+self.fn)
		sum2 = float(self.tn+self.fp)
		sum3 = float(self.tp+self.fp)
		sum4 = float(self.tn+self.fp)
		
		self.se = self.sp = self.acc = self.med = self.precision = self.fallout = self.FMeasure = 0.0
		
		if sum1!=0:
			self.se = (self.tp * 100.0) / sum1		
		if sum2!=0:			
			self.sp = (self.tn * 100.0) / sum2
		if (sum1+sum2)!=0:
			self.acc = ((self.tp + self.tn) * 100.0) / (sum1+sum2)		
		self.med = (self.se+self.sp)/2
		if sum3!=0:
			self.precision = (self.tp * 100.0) / sum3
		if sum4!=0:
			self.fallout = (self.fp * 100.0) / sum4
		if self.precision + self.se!=0:
			self.FMeasure = (2 * self.precision * self.se)/float(self.precision + self.se)
	def __repr__(self):
		return "%4d|TP:%5d |TN:%5d |FN:%5d |FP:%5d |Se:%3.2lf|Sp:%3.2lf|Acc:%3.2lf|Med:%3.2lf|"%((self.Iteration+1),int(self.tp),int(self.tn),int(self.fn),int(self.fp),self.se,self.sp,self.acc,self.med)
	def __iadd__(self,newObj):
		self.tp += newObj.tp
		self.tn += newObj.tn
		self.fp += newObj.fp
		self.fn += newObj.fn
		return self
	def __add__(self,newObj):
		self.tp += newObj.tp
		self.tn += newObj.tn
		self.fp += newObj.fp
		self.fn += newObj.fn	
		return self
class AttributeList:
	_attr = {}
	_error = ""
	_fileName = ""
	def __init__(self):
		self._attr = {}
		self._error = ""
		self._fileName = ""
	def Set(self,dictionar):
		self._attr = {}
		for item in dictionar:
			self._attr[item.lower()] = dictionar[item]		
		self._error = ""	
	def Add(self,dictionar):
		for item in dictionar:
			self._attr[item.lower()] = dictionar[item]		
		self._error = ""	
	def __iter__(self):
		return self._attr.__iter__()
	def next(self):
		return self._attr.next()	
	def __len__(self):
		return len(self._attr)
	def __getitem__(self,attrName):
		if attrName.lower() in self._attr:
			return self._attr[attrName.lower()]
		return None
	def __setitem__(self,key,value):
		self._attr[key] = value
	def __delitem__(self,attrName):
		if attrName.lower() in self._attr:
			del(self._attr[attrName.lower()])
	def __contains__(self,attrName):
		if attrName.lower() in self._attr:
			return True
		return False
	def __repr__(self):
		return str(self._attr)
	def __ConvertToValue(self,typeName,value):
		if typeName=="(STRING)":
			return value[1:-1]
		if typeName=="(BOOL)":
			if (value.lower()=="true") or (value.lower()=="yes"):
				return True
			elif (value.lower()=="false") or (value.lower()=="no"):
				return False
		if typeName.startswith("(UINT") or typeName.startswith("(INT"):
			return int(value.strip())
		if typeName=="(DOUBLE)" or typeName=="(FLOAT)":
			return float(value.strip())			
		return None
	def __GetTypeName(self,value):
		if value.startswith("\"") and value.endswith("\""):
			return "(STRING)"
		if value.startswith("'") and value.endswith("'"):
			return "(STRING)"
		if (value.lower()=="true") or (value.lower()=="yes") or (value.lower()=="false") or (value.lower()=="no"):
			return "(BOOL)"
		# numere intregi
		try:
			i = int(value.strip())
			if value.startswith("-"):
				return "(INT32)"
			return "(UINT32)"
		except:
			pass
			
		# valori reale	
		try:
			f = float(value.strip())
			return "(DOUBLE)"
		except:
			pass		
		return ""	
	def __AnalizeLine(self,line):
		t_name = ""		
		t_extra = ""
		if line.startswith("("):
			t_name = line.split(")",1)[0].upper()+")"
			line = line.split(")",1)[1].strip()
		if line.startswith("[") and line.endswith("]"):
			line = line[1:-1].strip()
			t_extra = "(ARRAY)"
		if len(t_name)==0:
			t_name = self.__GetTypeName(line)
		if (len(t_extra)==0) and (len(t_name)!=0):
			return self.__ConvertToValue(t_name,line)
		if t_extra == "(ARRAY)":
			list = line.split(",")
			if len(t_name)==0:
				return None
			res_l = []
			for item in list:
				res_l+=[self.__ConvertToValue(t_name,item)]
			return res_l
			
		# daca nu imi dau seama ce este , este string
		return line.strip()
	def GetError(self):
		return self._error
	def Load(self,fname):
		is_metadata = False
		self._error = ""
		try:
			for line in open(fname,"rt"):
				line = line.strip()
				if line.startswith(";") or line.startswith("#"):
					continue
				if line.startswith("[") and line.endswith("]"):
					continue
				if line.startswith("["):
					is_metadata = True
					continue
				if (line.endswith("]")) and (is_metadata==True):
					is_metadata = False
					continue
				if (is_metadata):
					continue
				if "=" in line:
					(k,v) = line.split("=",1)
					k = k.strip().lower()
					v = v.strip()
					if k in self._attr:
						self._error = "Key "+k+" already in list !"
						return False
					if len(v)==0:
						self._error = "Invalid value in line : "+line
						return False
					res = self.__AnalizeLine(v)
					if res==None:
						self._error = "Invalid format in line : "+line
						return False
					self._attr[k] = res					
			self._fileName = fname
			return True
		except Exception,e:
			self._error = str(e)
			return False
	def Save(self,fname=""):
		if fname=="":
			fname = self._fileName
		if CreateTemplateFile(self._attr,fname):
			self._fileName = fname;
			return True
		else:
			return False
	def Reload(self):
		if len(self._fileName.strip())==0:
			return False
		return self.Load(self._fileName)
	def GetFileName(self):
		return self._fileName
