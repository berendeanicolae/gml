import attributelist
import ialgorithm
import builder
import sys
if not len(sys.argv) > 1:
	print("You must pass the path of database!")
	sys.exit()
path = sys.argv[1]

attr = attributelist.AttributeList()
attr.AddString("DbName","SimpleTextFileDB")
attr.AddString("DbConnString","FileName={}".format(path))
attr.AddString("Conector","BitConnector{Table=RecordTable}")
attr.AddString("Notifyer","ConsoleNotifyer")
attr.AddDouble("LearningRate",0.02)
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
		print("Result of algorithm initialization: %d"%_in)
		if _in:
			print("alg result: ", _in)
			ialg.Execute("train");
			ialg.Wait();










