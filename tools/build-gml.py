import sys, os, shutil, glob, zipfile, re

devenv          = "\"C:\Program Files\Microsoft Visual Studio 10.0\Common7\IDE\devenv.exe\""
svn_checkout    = "svn checkout http://gml.googlecode.com/svn/trunk/ gml-read-only"
solution_name   = ".\gml-read-only\prj\gml\gml.sln"
config_name     = "Release"
ignore_ext      = ["pdb","exp","lib"]
projects = []

def checkout ():    
    print ("checking out the source tree")
    os.system(svn_checkout)
    
def build_solution():            
    for prj in projects:
        devcmd = "%s %s /Rebuild %s /Project %s"%(devenv,solution_name,config_name, prj)        
        print ("building with command: %s"%devcmd)
        os.system(devcmd)
    
    print ("build has terminated - checking for integrity of build")
    
def set_environ():
    os.system("cmd /c set PYTHON_INCLUDE='C:\Python31\include'") 
    os.system("cmd /c set PYTHON_LIB='gml-read-only\lib\python31.lib'")
    os.system("cmd /c set PYTHON_PATH='C:\Python31'")    

def addFolderToZip(myZipFile,folder):    
    print (folder)    
    for file in glob.glob(folder):
        print (file)
        if os.path.isfile(file):
            if file[-3:] in ignore_ext: continue
            isinstance(myZipFile, zipfile.ZipFile)            
            myZipFile.write(file, file, zipfile.ZIP_DEFLATED)
        elif os.path.isdir(file):
            addFolderToZip(myZipFile,file+"\*")    
    
def pack_zip():
    myZipFile = zipfile.ZipFile("gml-binary-release.zip", "w" )
    d = os.getcwd()
    os.chdir('gml-read-only\prj\gml\Release')
    addFolderToZip(myZipFile, '*')
    os.chdir(d)
    
def get_project_list ():
    global projects    
    lines = open (solution_name,"rt").readlines()
    for line in lines:
        #print (line)
        m = re.match("Project.*\".*= \"(\S+)\"", line)
        v = re.search("vcxproj", line)
        if m and v:
            print (m.group(1))
            projects.append(m.group(1))
            
    
set_environ()
checkout()
get_project_list()
build_solution()
pack_zip()