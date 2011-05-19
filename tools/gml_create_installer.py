import os,sys,shutil

arcList = ""

def AddFileToarchList(fname,arcname):
	global arcList
	nm = (os.path.basename(fname)+" "*256)[:32]
	if not os.path.exists(fname):
		print ("Missing "+fname)
		return True
	print("Adding "+nm+" => "+arcname)
	arcList+=(fname+"|"+arcname+"\n")
	return True
def AddFromFolderByExt(foldername,extlist,arcfoldername):	
	for r,d,files in os.walk(foldername):
		for name in files:
			if not "." in name:
				continue
			myext = name.lower().rsplit(".",1)[1]
			if myext in extlist:
				AddFileToarchList(os.path.join(r,name),os.path.join(arcfoldername,r[len(foldername):],name))		
def find_revision():
	try:
		os.system("svn info -r HEAD >svn_info.txt")
	except:
		print("Unable to find curent revision...")
		return ""
	try:
		for line in open("svn_info.txt","r"):
			if line.startswith("Revision:"):
				return "r"+line.split(":",1)[1].strip()
	except:
		print("Unable to load curent revision...")
		return ""
def main():
	global arcList
	rev = find_revision()
	if len(rev)==0:
		rev = "UNK"
	zname = "GMLInstaller_"+rev+".exe"
	print ("Building: "+zname) 	
	base_folder = "../prj/gml/Release"
	ia_base_folder = "../prj/GMLFramework/Release"	
	#plugin
	AddFromFolderByExt(os.path.join(base_folder,"Algorithms"),["alg"],"Algorithms")
	AddFromFolderByExt(os.path.join(base_folder,"Notifiers"),["ntf"],"Notifiers")
	AddFromFolderByExt(os.path.join(base_folder,"DataBases"),["db"],"DataBases")
	AddFromFolderByExt(os.path.join(base_folder,"Connectors"),["dbc"],"Connectors")
	#lib & exe
	AddFileToarchList(os.path.join(base_folder,"gmllib.dll"),"gmllib.dll")
	AddFileToarchList(os.path.join(base_folder,"libmysql.dll"),"libmysql.dll")
	AddFileToarchList(os.path.join(base_folder,"gml.exe"),"gml.exe")	
	#SDK
	AddFileToarchList("../lib/gmllib.lib","SDK/gmllib.lib")
	AddFileToarchList("../lib/gmllib.h","SDK/gmllib.h")
	#graphics
	#lib & exe
	AddFileToarchList(os.path.join(ia_base_folder,"gmllib.dll"),"gmllib.dll")
	AddFileToarchList(os.path.join(ia_base_folder,"libmysql.dll"),"libmysql.dll")
	AddFileToarchList(os.path.join(ia_base_folder,"gml.exe"),"gml.exe")	
	#gmlpy
	AddFileToarchList("gmlpy.py","gmlpy.py")		
	#curat alte chestii
	open("arcList.txt","wt").write(arcList);
	shutil.copy("../prj/GMLInstaller/Release/GMLInstaller.exe",zname)
	os.system("GDropperCreator.exe "+zname+" arcList.txt")
	try:
		os.unlink("svn_info.txt")
		os.unlink("arcList.txt")
	except:
		pass
	
main()
