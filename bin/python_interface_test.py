import gmllib
import sys
if not len(sys.argv) > 1:
	print("You must pass the path of database!")
	sys.exit()
path = sys.argv[1]

ialg = gmllib.CreateAlgorithm("GDTPerceptrons")
print("Inited algorithm: {}".format(ialg)) 

if not ialg:
	print ("could not init the algorithm")
	sys.exit()

config = ialg.Set({"Name":"MCU-Testing", 
                   "DataBase":"SimpleTextFileDB{FileName=%s}"%path,
		   "Conector":"BitConnector{Table=RecordTable",
		   "Notifier":"ConsoleNotifier{useColors=True}",
		   "LearningRate":0.02,
		   "MaxIterations":100,
                   "InitialWeight":"random",
                   "useBias":False,
                   "SaveData":"AfterEachIteration",
                   "SaveBest":"BestACC"})

if config:
	_in = ialg.Init()
	print("Result of algorithm initialization: %d"%_in)
	if _in:
		ialg.Execute("train");
		ialg.Wait();











