import os,sys

comp = "cl.exe "
output_folder = ""

def BuildGmlLib_h(c_path,f_path):
	list_headers = [	
		"GTVector.h",
		"Vector.h",
		"BitSet.h",
		"Matrix.h",
		"FixMatrix.h",
		"GTFVector.h",
		"Indexes.h",	
		"md5.h",
		"GString.h",
		"File.h",
		"Timer.h",
		"AlgorithmResult.h",
		"IParalelUnit.h",
		"ThreadParalelUnit.h",
		"AttributeList.h",
		"GMLObject.h",
		"INotifier.h",
		"VectorOp.h",
		"DBRecord.h",
		"MLRecord.h",
		"IDataBase.h",
		"IConnector.h",
		"IAlgorithm.h",	
		"Builder.h",
		"IMLAlgorithm.h",		
	]
	print("Files:",c_path)
	print("Dest :",f_path)
	f = open(os.path.join(f_path,"gmllib.h"),"w")
	f.write("#ifndef __GML_LIB__\n#define __GML_LIB__\n\n")
	f.write(open(os.path.join(c_path,"compat.h"),"r").read())
	f.write("\n\n//-----------------------------------------------------------------------------------------------------------------------\n\n")
	#f.write("namespace GML {\n");
	for name in list_headers:
		print("Adding ... "+name)
		f.write("//===================== "+name+" =================================================================================\n");
		for line in open(os.path.join(c_path,name),"rt"):
			if "#include" in line:
				continue
			if "#pragma once" in line:
				continue
			#if "namespace GML" in line:
			#	continue
			if (" EXPORT " in line) and ("class" in line):
				line = line.replace("EXPORT","")
			if (" EXPORT " in line) and ("struct" in line):
				line = line.replace("EXPORT","")
			f.write(line)
		f.write("\n")
	#f.write("};\n")
	f.write("#endif\n\n")
	f.close()

def LoadCompileInfoFile(fname):
	try:
		d = {}
		for line in open(fname,"rb"):
			line = line.strip()
			if len(line)==0:
				continue
			if line.startswith(";"):
				continue
			if line.startswith("#"):
				continue
			if "=" in line:
				d[line.split("=",1)[0].strip().lower()] = line.split("=",1)[1].strip()
		return d
	except:
		print("Error reading from : "+fname)
		return None
def GetWindowsCompileString_x86(fname):
	d = LoadCompileInfoFile(fname)
	if d==None:
		return None
	dname = os.path.dirname(fname)
	c = comp+" /I\""+os.path.join(dname,"Inc")+"\" /DOS_WINDOWS "
	if "extra" in d:
		c = c + d["extra"]+" "
	if not "sources" in d:
		print("Missing 'sources' field in "+fname)
		return None	
	if not "out" in d:
		print("Missing 'out' field in "+fname)
		return None	
	list = d["sources"].split(" ")
	for i in list:
		if len(i)!=0:
			c+=" \""+os.path.join(dname,"Src",i)+"\" "
	c = c+" /link /NOLOGO /DLL /MANIFEST:NO /SUBSYSTEM:WINDOWS /OUT:\""+os.path.join(output_folder,d["out"])+"\" "
	return c
def CreateFolders():
	try:	
		for root, dirs, files in os.walk(output_folder, topdown=False):
		    for name in files:
		        os.remove(os.path.join(root, name))
		        #print os.path.join(root, name)
		    for name in dirs:
		        os.rmdir(os.path.join(root, name))
		        #print os.path.join(root, name)
		os.mkdir(os.path.join(output_folder,"Algorithms"))
		os.mkdir(os.path.join(output_folder,"Connectors"))
		os.mkdir(os.path.join(output_folder,"Databases"))
		os.mkdir(os.path.join(output_folder,"Notifiers"))
		os.mkdir(os.path.join(output_folder,"SDK"))
		return True
	except:
		print("Unable to create folder on : "+output_folder)
		return False

def main():
	output_folder = sys.argv[2]
	if CreateFolders()==False:
		return			
	c = GetWindowsCompileString_x86(sys.argv[1])
	os.system(c)
	
main()
				
