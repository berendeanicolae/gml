import attributelist
import ialgorithm
import builder
import sys
if not len(sys.argv) > 1:
	print("You must pass the path of database!")
	sys.exit()
path = sys.argv[1]

attr = attributelist.AttributeList()

attr.Set({ "DbName":"SimpleTextFileDB{FileName=%s}"%path,
	   "Conector":"BitConnector{Table=RecordTable}",
	   "Notifyer":"ConsoleNotifyer{useColors=True}",
	   "LearningRate":0.02,
	   "MaxIteratii":100})

b = builder.Builder()

ialg = b.CreateAlgorithm("SimplePerceptronAlgorithm")
print("Inited algorithm: {}".format(ialg)) 
if not ialg:
	pass
else:
	config = ialg.SetProperty(attr)
	if config:
		_in = ialg.Init()
		print("Result of algorithm initialization: %d"%_in)
		if _in:
			ialg.Execute("train");
			ialg.Wait();











