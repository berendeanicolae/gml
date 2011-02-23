import gml
import sys
if not len(sys.argv) > 1:
	print("You must pass the path of database!")
	sys.exit()
path = sys.argv[1]

b = gml.Builder()

ialg = b.CreateAlgorithm("SimplePerceptronAlgorithm")
print("Inited algorithm: {}".format(ialg)) 

if not ialg:
	print ("could not init the algorithm")
	sys.exit()

config = ialg.Set({"DbName":"SimpleTextFileDB{FileName=%s}"%path,
		   "Conector":"BitConnector{Table=RecordTable}",
		   "Notifyer":"ConsoleNotifyer{useColors=True}",
		   "LearningRate":0.02,
		   "MaxIteratii":100})

if config:
	_in = ialg.Init()
	print("Result of algorithm initialization: %d"%_in)
	if _in:
		ialg.Execute("train");
		ialg.Wait();











