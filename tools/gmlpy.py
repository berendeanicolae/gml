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
