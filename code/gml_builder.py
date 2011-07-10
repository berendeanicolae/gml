import os,sys,subprocess

output_folder = ""
compile_mode = ""

#win32 , win64

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
	print("Creating ... "+os.path.join(f_path,"gmllib.h"))
	try:
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
		return True
	except:
		print("Unable to greate gmllib.h -> abording ...")
		return False
def LoadCompileInfoFile(fname):
	try:
		d = {}
		d["compilefile"] = fname
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
def BuildDLLMainForPlugins(compileDict):
	libname = ""
	owner = "Unknown"
	version = "1"
	comments = ""
	
	if "libname" in compileDict:
		libname = compileDict["libname"]
	else:
		print("Missing 'libname' field in .compile file ... abording ")
		return False
	if "owner" in compileDict:
		owner = compileDict["owner"]
	if "version" in compileDict:
		version = compileDict["version"]
	if "comments" in compileDict:
		comments = compileDict["comments"]
	try:
		f = open("dllmain.cpp","w")
		f.write("#include \""+libname+".h\"\n\n")
		f.write("LIB_INTERFACE("+libname+",\""+owner+"\","+version+",\""+comments.replace("\"","'")+"\")\n")
		f.close()
		return True
	except Exception,e:
		print("Unable to create DLLMain for "+libName)
		print("Exception: "+str(e))
		return False
def BuildPluginName(compileDict):
	fname = os.path.basename(compileDict["compilefile"]).split(".",1)[0]
	if "algorithms" in compileDict["compilefile"].lower():
		compileDict["out"] = os.path.join("Algorithms",fname+".alg")
		return True
	if "connectors" in compileDict["compilefile"].lower():
		compileDict["out"] = os.path.join("Connectors",fname+".dbc")
		return True	
	if "database" in compileDict["compilefile"].lower():
		compileDict["out"] = os.path.join("Databases",fname+".db")
		return True	
	if "notifiers" in compileDict["compilefile"].lower():
		compileDict["out"] = os.path.join("Notifiers",fname+".ntf")
		return True	
	print("Unable to locate type for :"+compileDict["compilefile"])
	print("It should be located in one the following directories: Algorithms,Connectors,DataBases or Notifiers")
	return False
		
def GetWindowsCompileString(fname,x64):
	global output_folder,cl_32,cl_64
	d = LoadCompileInfoFile(fname)
	if d==None:
		return None
	dname = os.path.dirname(fname)
	gmlplugin = False
	usegmllib = False
	if ("gmlplugin" in d) and (d["gmlplugin"]=="yes"):
		gmlplugin = True
	if ("usegmllib" in d) and (d["usegmllib"]=="yes"):
		usegmllib = True
		
	c = "cl.exe /I\""+os.path.join(dname,"Inc")+"\" /DOS_WINDOWS /EHsc /fp:precise /nologo /O2 /Ob2 /Ot "
	if gmlplugin or usegmllib:
		c+=" /I\""+os.path.join(output_folder,"SDK")+"\" "		
	if "extra" in d:
		c = c + d["extra"]+" "
	if not "sources" in d:
		print("Missing 'sources' field in "+fname)
		return None	
	if gmlplugin:
		if BuildDLLMainForPlugins(d)==False:
			return None
		if BuildPluginName(d)==False:
			return None
		c+=" dllmain.cpp "
	else:
		if not "out" in d:
			print("Missing 'out' field in "+fname)
			return None	

	list = d["sources"].split(" ")
	for i in list:
		if len(i)!=0:
			c+=" \""+os.path.join(dname,"Src",i)+"\" "
	c = c+" /link /NOLOGO /MANIFEST:NO /OUT:\""+os.path.join(output_folder,d["out"])+"\" "
	if gmlplugin or usegmllib:
		c+=" \""+os.path.join(output_folder,"SDK","gmllib.lib")+"\" "	
	if "libs" in d:
		c+=" "+d["libs"]+" "		
	if ("exe" in d) and (d["exe"]=="yes"):
		c+=" /SUBSYSTEM:CONSOLE "
	else:
		c+=" /SUBSYSTEM:WINDOWS /DLL "
	if x64 == True:
		c+= " /MACHINE:X64 "
	else:
		c+= " /MACHINE:X86 "
	return c
def Compile(fname):
	global compile_mode,cl_32,cl_64
	print("="*30+" Compile:"+os.path.basename(fname).split(".",1)[0]+" "+"="*30)
	if compile_mode=="win32":
		c = GetWindowsCompileString(fname,False)
	elif compile_mode=="win64":
		c = GetWindowsCompileString(fname,True)
	else:
		print("Invalid compile mode : "+compile_mode)
		return False
	if (c==None):
		return False	
	#try:
	#	open("run.bat","w").write("@"+c)
	#	if os.system("run.bat")==0:
	#		print("Error running command : "+c)
	#		return False
	#except:
	#	print("Error running command : "+c)
	#	return False
	
	#print cl_32
	#print c	
	#if os.system(cl_32+c)!=0:
	#if subprocess.call([cl_32+c,c])!=0:
	#	print("Error running command : "+c)
	#	return False
	#p = subprocess.Popen(c,executable=cl_32, shell=True)
	#sts = os.waitpid(p.pid, 0)[1]
	if os.system(c)!=0:
		print("Error running command : "+c)
		return False
	return True
def CreateFolders():
	global output_folder
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
def BuildFromFolder(folderName):
	try:
		for root, dirs, files in os.walk(folderName):
		    for name in files:
				if (name.lower().endswith(".compile")) and (".svn" not in root.lower()):
					if Compile(os.path.join(root,name))==False:
						return False
		return True
	except Exception,e:
		print("Error browsing in "+folderName)
		print("Exception: "+str(e))
		return False
def BuildGMLLib():
	global output_folder
	if Compile("./GMLLIb/gmllib.compile")==False:
		return False
	if BuildGmlLib_h("./GMLLIb/Inc",os.path.join(output_folder,"SDK"))==False:
		return False
	#mut si lib-ul
	try:
		os.rename(os.path.join(output_folder,"gmllib.lib"),os.path.join(output_folder,"SDK","gmllib.lib"))
	except:
		print("Error moving "+os.path.join(output_folder,"gmllib.lib")+" to SDK folder !")
		return False
	return True	
def BuildGMLEXE():
	return Compile("./GML/gml.compile")
def help():
	print("""
Build.py - builder for gml core
By Gavrilut Dragos @2011
Usage:
	Build.py <Platform> <OutputFolder>
Where:
	<Platform>       is one of the followings: Win32
	<OutputFolder>   is the folder where gml core will be created
""")
def CleanUpOutputFolder():
	global output_folder
	try:
		for root, dirs, files in os.walk(output_folder):
		    for name in files:
				nm = name.lower()
				fnm = os.path.join(root,name)
				do_delete = False
				if nm.endswith(".exp"):
					do_delete = True
				if (nm.endswith(".lib")) and ("gmllib.lib" not in nm):
					do_delete = True
				if do_delete:
					os.unlink(fnm)				
		return True
	except Exception,e:
		print("Unable to cleanup :"+output_folder)
		print("Exception: "+str(e))
		return False
def CleanUpCurrentFolder():
	try:
		for name in os.listdir("."):
			if name.lower().endswith(".obj"):
				os.unlink(name)
		if os.path.exists("dllmain.cpp"):
			os.unlink("dllmain.cpp")
		return True
	except Exception,e:
		print("Unable to cleanup current folder ")
		print("Exception: "+str(e))
		return False		
def main():
	global output_folder,compile_mode
	if len(sys.argv)!=3:
		help()
		return
	output_folder = sys.argv[2]
	compile_mode = sys.argv[1].lower()
	if CleanUpCurrentFolder()==False:
		return
	if CreateFolders()==False:
		return			
	if BuildGMLLib()==False:
		return
	if BuildGMLEXE()==False:
		return
	if BuildFromFolder("./Notifiers")==False:
		return
	if BuildFromFolder("./Connectors")==False:
		return
	if BuildFromFolder("./DataBasePlugins")==False:
		return
	if BuildFromFolder("./Algorithms")==False:
		return	
	print("=============================== CLEAN UP ================================")
	CleanUpCurrentFolder()
	CleanUpOutputFolder()
	print("=============================== ALL OK ==================================")		
main()
				
