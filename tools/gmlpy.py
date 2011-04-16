import os,time,random

#Fill with the path for GML Package
GML_PATH = r"E:\lucru\GML\gml\prj\gml\Release"

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
	
def __ListToString(lista):
	count_dict = 0
	count_str = 0
	for item in lista:
		if __GetVarType(item) == 'dict':
			count_dict += 1
		if __GetVarType(item) == 'str':
			count_str += 1
	if count_dict==len(lista):
		return __ListOfDictToString(lista)
	if count_str==len(lista):
		return __ListOfStringsToString(lista)
	print("[Error] Lists should contain dict elements or string elements only")
	return None	
def __CreateTemplateFile(dictionar,fname):
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
			s+="\""+r+"\"\n"
		else:
			print("[Error] Unknwon type for "+k+" => "+tip)
			return False
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
	if __CreateTemplateFile(dictionar,fname)==False:
		return False
	os.system(os.path.join(GML_PATH,"gml.exe")+" run "+fname)
	try:
		os.unlink(fname)
	except:
		pass
	return True		
