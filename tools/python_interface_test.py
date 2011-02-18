import attributelist
import ialgorithm
import builder
import sys
if not len(sys.argv) > 1:
	print("You must pass the path of database!")
	sys.exit()
path = sys.argv[1]

attr = attributelist.AttributeList()
attr.AddString("DbName","Sqlite")
attr.AddString("DbConnString","FileName={}".format(path))
attr.AddString("Conector","FullCacheConnector")
attr.AddString("Notifyer","ConsoleNotifyer")
attr.AddDouble("LearningRate",0.01)
attr.AddInt32("MaxIteratii",100, "")
b = builder.Builder()

ialg = b.CreateAlgorithm("SimplePerceptronAlgorithm","")
print("Inited algorithm: {}".format(ialg)) 
if not ialg:
	pass
else:
	config = ialg.SetConfiguration(attr)
	if config:
		_in = ialg.Init()
		print("Result of algorithm initialization: {}".format(_in))
		if _in:
			print("alg result: ", _in)
			a = 0.0
			#ialgorithm.IAlgorithm_Execute(a)
			ialg.Execute(a)   









