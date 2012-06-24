import os, gmlpy, math, glob, shutil, ast, time

# global data
trainDataSet = ""
testDataSet = ""
connectorType = ""
KernelParamDouble = 0.0
KernelParamInt = 0.0

running_file_obj = None

def debug_print (msg):
	global running_file_obj
	dct = getParameterConfig()
	if not running_file_obj:
		running_file_obj = open (dct["BlockFilePrefix"]+"-running.winsize=%d.log"%(dct["WindowSize"]), "wt")
	
	timestamp = time.time()
	running_file_obj.write("time:"+str(int(timestamp)) + ":" + msg+"\n")
	running_file_obj.flush()

def getParameterConfig():
	param = {
	    "AlgorithmName" : "ProbVectorMachine",
	    "KernelType" : "Scalar",
	    
	    # these two are set outside of this function	    
	    #"KernelParamDouble" : 0.0, #first param
	    #"KernelParamInt" : 3, #second param

	    "BlockFileSize" : 1,
	    "BlockStart" : 0,
	    "BlockCount" : 1,
	    "BlockFilePrefix" : "mushroom",

	    "ThreadsCount" : 2,

	    #training related vars
	    "Lambda" : 1.4,
	    "WindowSize" : 2, 
	    "NrUpdatesPerNormBlock" : 20,
	    "NrUpdatesPerLastBlock" : 5,
	    "T" : 1.0
	} 
	return param

def getConnectorDict ():
	con = {
	    "DataFileName":trainDataSet,
	    "Type":connectorType
	}
	return con

def getConnectorTestDict ():
	con = {
	    "DataFileName":testDataSet,
	    "Type":connectorType
	}
	return con


def getNotifierDict ():
	notif = {
	    "UseColors":True,
	    "Type":"NullNotifier"
	}
	return notif

def ExecuteCommand(cmd, itNumber=0): 
	global KernelParamInt, KernelParamDouble
	line50 = '-'*50
	#debug_print ("%s%s%s"%(line50, cmd, line50))
	
	mydict = getParameterConfig()
	mydict["Connector"] = getConnectorDict()
	mydict["Notifier"] = getNotifierDict()
	mydict["ConnectorTest"] = getConnectorTestDict()

	mydict["Command"] = cmd
	mydict["IterationNo"] = itNumber
	
	mydict["KernelParamInt"] = KernelParamInt
	mydict["KernelParamDouble"] = KernelParamDouble

	gmlpy.Run(mydict)    

def MakeInitialPrep():    
	ExecuteCommand("PreCompGramMatrix")
	ExecuteCommand("MergeKPrimeFiles")
	ExecuteCommand("PreCompEqNorm")
	ExecuteCommand("InitStateVars")    

def PerformTrain4Iter (itNumber, varT=1):   

	mydict = getParameterConfig()
	con = getConnectorDict()
	testCon = getConnectorTestDict()
	notif = getNotifierDict()

	mydict["Connector"] = con
	mydict["Notifier"] = notif
	mydict["ConnectorTest"] =  testCon

	mydict["IterationNo"] = itNumber
	mydict["T"] = varT

	mydict["Command"] = "BlockTraining"
	line50 = '-'*50
	#debug_print ("%s %s it: %d %s"%(line50, mydict["Command"], itNumber, line50))        
	gmlpy.Run(mydict)     

	mydict["Command"] = "LastBlockTraining"
	line50 = '-'*50
	#debug_print ("%s %s it: %d %s"%(line50, mydict["Command"], itNumber, line50))        
	gmlpy.Run(mydict)     


def IterateTraining (nrIter, varT, nrIterStable):
	paramDict = getParameterConfig()
	score_vect = []

	debug_print ("performing training for T = %.05f"%varT)
	retValue = False
	lastIt = 0

	for it in range (1, nrIter):		
		PerformTrain4Iter(it, varT)    
		ExecuteCommand("GatherBlockStates",it)
		lastIt = it

		obj = open("%s.state.iter.%04d.block.score"%(paramDict["BlockFilePrefix"],it))
		score = obj.read().strip()
		obj.close()
		score = float(score)
		score_vect.append(score)		
		debug_print("iteration:%04d;score:%.07f"%(it,score))

		shutil.copy("%s.state.iter.%04d.block.score"%(paramDict["BlockFilePrefix"],it), "%s.state.score.current"%(paramDict["BlockFilePrefix"]))        

		# remove garbage
		if it>1:
			files2rem = glob.glob("%s.state.iter.%04d.block.????"%(paramDict["BlockFilePrefix"],it-1))
			files2rem += glob.glob("%s.state.iter.%04d.block.last"%(paramDict["BlockFilePrefix"],it-1))
			files2rem += glob.glob("%s.state.iter.%04d.block.score"%(paramDict["BlockFilePrefix"],it-1))
			files2rem += glob.glob("%s.state.iter.%04d.block.all"%(paramDict["BlockFilePrefix"],it-1))
			for item in files2rem:
				#debug_print ("removing file: %s"%item)
				try:os.unlink(item)#pass#
				except: pass       

		if score == 0:                  
			shutil.copy("%s.state.iter.%04d.block.all"%(paramDict["BlockFilePrefix"],it), "%s.state.vart.%05.05f.all"%(paramDict["BlockFilePrefix"],varT))
			debug_print ("yyeeeeeyyyy")
			retValue = True
			break
		elif it > nrIterStable:
			if min(score_vect[0:len(score_vect)-nrIterStable]) - min(score_vect[-nrIterStable:]) < 0.001:
				break

	lastIt += 1
		
	ExecuteCommand("BlockScoreComputation", lastIt) 
	ExecuteCommand("LastBlockTraining", lastIt) 
	ExecuteCommand("GatherBlockStates", lastIt) 

	obj = open("%s.state.iter.%04d.block.score"%(paramDict["BlockFilePrefix"],lastIt))
	score = float(obj.read().strip())
	obj.close()        

	lastIt -= 1    

	if score < 0.0001 :
		shutil.copy("%s.state.iter.%04d.block.all"%(paramDict["BlockFilePrefix"],lastIt), "%s.state.iter.%04d.block.all"%(paramDict["BlockFilePrefix"], 0))
		retValue = True
	else:
		retValue = False    
		
	debug_print("realscore:%.07f"%score)

	files2rem = glob.glob("%s.state.iter.%04d.block.????"%(paramDict["BlockFilePrefix"],lastIt))
	files2rem += glob.glob("%s.state.iter.%04d.block.last"%(paramDict["BlockFilePrefix"],lastIt))
	files2rem += glob.glob("%s.state.iter.%04d.block.score"%(paramDict["BlockFilePrefix"],lastIt))
	files2rem += glob.glob("%s.state.iter.%04d.block.all"%(paramDict["BlockFilePrefix"],lastIt))    

	lastIt += 1
	files2rem += glob.glob("%s.state.iter.%04d.block.????"%(paramDict["BlockFilePrefix"],lastIt))
	files2rem += glob.glob("%s.state.iter.%04d.block.last"%(paramDict["BlockFilePrefix"],lastIt))
	files2rem += glob.glob("%s.state.iter.%04d.block.score"%(paramDict["BlockFilePrefix"],lastIt))
	files2rem += glob.glob("%s.state.iter.%04d.block.all"%(paramDict["BlockFilePrefix"],lastIt))    

	for item in files2rem:
		#debug_print ("removing file: %s"%item)
		try:os.unlink(item)#pass#
		except: pass       

	return retValue


def BinarySearchTraining ():
	varTLeft  = 0.0
	varTRight = 0.0
	epsilon = 0.001
	maxNrIter = 5000
	nrIterStable = 50

	foundFeasible = False

	for pow2 in range (0, 10):
		varTLeft = varTRight
		varTRight = 2**pow2 + 1
		if IterateTraining(maxNrIter, varTRight, nrIterStable):
			foundFeasible = True            
			return
			break
		return
	if not foundFeasible:
		debug_print ("cannot perform training on this database. sorry")
		exit(1)

	nrIterStable *= 3

	while varTRight-varTLeft > epsilon:
		varTCenter = (varTLeft + varTRight)/2
		if IterateTraining(maxNrIter, varTCenter, nrIterStable):
			varTRight = varTCenter
		else:
			varTLeft = varTCenter		
	debug_print ("found T = %.05f"%varTRight)


def PerformClassification():
	mydict = getParameterConfig()
	con = getConnectorDict()
	testCon = getConnectorTestDict()
	notif = getNotifierDict()

	mydict["Connector"] = con
	mydict["Notifier"] = notif
	mydict["ConnectorTest"] =  testCon        

	mydict["Command"] = "Clasify"
	mydict["ModelFile"] = "pvm.state.iter.0000.block.all"

	gmlpy.Run(mydict)
	resultFile = "%s-classification-result.txt"%(mydict["BlockFilePrefix"])
	if os.path.exists(resultFile):
		dictStr = open(resultFile).read().strip()
		results = ast.literal_eval(dictStr)
		return float(results["acc"])
	return -1

def frange(start, end=None, inc=None):
	"A range function, that does accept float increments..."

	if end == None:
		end = start + 0.0
		start = 0.0

	if inc == None:
		inc = 1.0

	L = []
	while 1:
		next = start + len(L) * inc
		if inc > 0 and next >= end:
			break
		elif inc < 0 and next <= end:
			break
		L.append(next)

	return L

def PerformGridSearch():
	global KernelParamDouble, KernelParamInt
	
	# we will make simple grid search, with all the values inside defined by the followingn dict
	# Poly,Scalar,Rbf
	vals = {
	    "Poly": [{"start": 0.0, "stop": 10.0, "step":1.0, "func":lambda x:float(x) }, # KernelParamDouble that is: t
	             {"start": 1.0, "stop": 12.0, "step":2.0, "func":lambda x:int(x)}], # KernelParamInt that is: d
	    "Scalar": [],
	    "Rbf": [{"start": -20.0, "stop": 3.0, "step":1.0, "func":lambda x:float(pow(2,x))}] # KernelParamDouble that is: gamma
	}
	
	grid = []
	kernel = getParameterConfig()["KernelType"]	
	if len(vals[kernel])==2:
		for v1 in frange(vals[kernel][0]["start"], vals[kernel][0]["stop"], vals[kernel][0]["step"]):
			for v2 in frange(vals[kernel][1]["start"], vals[kernel][1]["stop"], vals[kernel][1]["step"]):
				grid.append((vals[kernel][0]["func"](v1), vals[kernel][1]["func"](v2)))
				
	elif len(vals[kernel])==1:
		for v1 in frange(vals[kernel][0]["start"], vals[kernel][0]["stop"], vals[kernel][0]["step"]):
			grid.append((vals[kernel][0]["func"](v1), int(0)))
			
	else:
		grid.append((float(0), int(0)))
		
	maxAcc = -1
	maxUple = ()
	for vd, vi in grid:
		KernelParamDouble = "%.07f"%vd
		KernelParamInt = "%d"%vi
		debug_print ("Grid search for KernelParamDouble= %s; KernelParamInt: %s"%(KernelParamDouble,KernelParamInt))
		MakeInitialPrep()    
		BinarySearchTraining()
		acc = PerformClassification()
			
		
if __name__ == "__main__":	
	
	trainDataSet = "mushroom.agaricus-lepiota.data.fullcache"
	testDataSet = "mushroom.agaricus-lepiota.data.fullcache.test"
	connectorType = "FullCacheConnector"	
	PerformGridSearch()		
	
	#BinarySearchTraining()	
	#IterateTraining(4000, 1.0, 100);