import sys,os

list_headers = [	
	"GTVector.h",
	"File.h",
	"md5.h",
	"GString.h",
	"Timer.h",
	"AlgorithmResult.h",
	"IParalelUnit.h",
	"ThreadParalelUnit.h",
	"DBRecord.h",
	"MLRecord.h",
	"IDataBase.h",
	"MLInterface.h",
	"INotify.h",
]

c_path = sys.argv[1]
f_path = sys.argv[2]

print "Files:",c_path
print "Dest :",f_path
f = open(os.path.join(f_path,"gmllib.h"),"wb")
f.write("#ifndef __GML_LIB__\n#define __GML_LIB__\n\n")
f.write(open(os.path.join(c_path,"compat.h"),"rb").read())
f.write("\n\n//-----------------------------------------------------------------------------------------------------------------------\n\n")
#f.write("namespace GML {\n");
for name in list_headers:
	print "Adding ... "+name
	f.write("//===================== "+name+" =================================================================================\n");
	for line in open(os.path.join(c_path,name),"rb"):
		if "#include" in line:
			continue
		if "#pragma once" in line:
			continue
		#if "namespace GML" in line:
		#	continue
		if (" EXPORT " in line) and ("class" in line):
			line = line.replace("EXPORT","")
		f.write(line)
	f.write("\n")
#f.write("};\n")
f.write("#endif\n\n")
f.close()
