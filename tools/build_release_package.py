import os,sys,zipfile

def AddFileToZip(zip,fname,arcname):
	nm = (os.path.basename(fname)+" "*256)[:32]
	if not os.path.exists(fname):
		print ("Missing "+fname)
		return True
	print("Adding "+nm+" => "+arcname)
	try:
		zip.write(fname,arcname)
		return True
	except:
		print("Error adding : "+fname)
		return False
def AddFromFolderByExt(zip,foldername,extlist,arcfoldername):	
	for r,d,files in os.walk(foldername):
		for name in files:
			if not "." in name:
				continue
			myext = name.lower().rsplit(".",1)[1]
			if myext in extlist:
				AddFileToZip(zip,os.path.join(r,name),os.path.join(arcfoldername,r[len(foldername):],name))		
def main():
	z = zipfile.ZipFile("GML-Package.zip","w",zipfile.ZIP_DEFLATED)
	base_folder = "../prj/gml/Release"
	#plugin
	AddFromFolderByExt(z,os.path.join(base_folder,"Algorithms"),["alg"],"Algorithms")
	AddFromFolderByExt(z,os.path.join(base_folder,"Notifiers"),["ntf"],"Notifiers")
	AddFromFolderByExt(z,os.path.join(base_folder,"DataBases"),["db"],"DataBases")
	AddFromFolderByExt(z,os.path.join(base_folder,"Connectors"),["dbc"],"Connectors")
	#lib & exe
	AddFileToZip(z,os.path.join(base_folder,"gmllib.dll"),"gmllib.dll")
	AddFileToZip(z,os.path.join(base_folder,"libmysql.dll"),"libmysql.dll")
	AddFileToZip(z,os.path.join(base_folder,"gml.exe"),"gml.exe")	
	#SDK
	AddFileToZip(z,"../lib/gmllib.lib","SDK/gmllib.lib")
	AddFileToZip(z,"../lib/gmllib.h","SDK/gmllib.h")
	z.close()
	
main()
