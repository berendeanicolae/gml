import os,sys

def Help():
	print """
create_plugin.py type name
Where:
	* type  		is algorithm , database , connector , notifier
	* name		the name of the plugin
A .compile file will be created as well	
"""
def CreateFolder(fname):
	try:
		os.mkdir(fname)
		return True
	except Exception,e:
		print("Unable to create: "+fname)
		print("Exception: "+str(e))
		return False
def CreateFile(fname,content):
	try:
		f = open(fname,"w")
		f.write(content)
		f.close()
		return True
	except Exception,e:
		print("Unable to create: "+fname)
		print("Exception: "+str(e))
		return False
def LoadHeaderTemplate(name):
	try:
		s = open(os.path.join(".","Templates",name+".HeaderTemplate"),"rt").read()
		return s
	except Exception,e:
		print("Unable to load header template: "+os.path.join(".","Templates",name+".HeaderTemplate"))
		print("Exception: "+str(e))
		return None
def LoadCodeTemplate(name):
	try:
		s = open(os.path.join(".","Templates",name+".CodeTemplate"),"rt").read()
		return s
	except Exception,e:
		print("Unable to load code template: "+os.path.join(".","Templates",name+".CodeTemplate"))
		print("Exception: "+str(e))
		return None		
def CreateAlgorithm(template_name):
	global template_alg_h,template_alg_cpp
	name = sys.argv[2]
	path = os.path.join(".","Algorithms",name)
	hFile = LoadHeaderTemplate(template_name)
	if hFile==None:
		return
	codeFile = LoadCodeTemplate(template_name)
	if codeFile==None:
		return
	if os.path.exists(path):
		print("Algorithm '"+name+"' already exists ...")
		return
	if CreateFolder(path)==False:
		return
	if CreateFolder(os.path.join(path,"inc"))==False:
		return
	if CreateFolder(os.path.join(path,"src"))==False:
		return
	if CreateFile(os.path.join(path,"inc",name+".h"),hFile.replace("$(NAME)",name))==False:
		return
	if CreateFile(os.path.join(path,"src",name+".cpp"),codeFile.replace("$(NAME)",name))==False:
		return	
	compileFile = "sources = "+name+".cpp\ngmlplugin = yes\nlibname = "+name+"\n"
	if CreateFile(os.path.join(path,name+".compile"),compileFile)==False:
		return
	print("All ok -> Algorithm "+name+" created !")
def main():
	if len(sys.argv)<3:
		Help()
		return
	if sys.argv[1]=="algorithm":
		CreateAlgorithm("Algorithm")
		return
main()
