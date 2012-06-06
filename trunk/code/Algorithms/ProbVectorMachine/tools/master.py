import os, sys, shutil, subprocess, time, tempfile, glob

slaves = [ ( "127.0.0.1"    , {"ThreadsCount":2, "BlockStart":0, "BlockCount": 2, "WindowSize":4} ),
           ( "192.168.1.109", {"ThreadsCount":2, "BlockStart":2, "BlockCount": 3, "WindowSize":4} ) 
        ]

workPackage = r"D:\GML\package"
#-----------------------------------------------------------------------------------------------------------

def rmdir(d):
	try:
		for path in (os.path.join(d,f) for f in os.listdir(d)):
			if os.path.isdir(path):
				shutil.rmtree(path)
			else:
				os.unlink(path)
	except Exception as exc:
		print (repr(exc))

def copyDirectoryTree(directory, destination, preserveSymlinks=True):
	for entry in os.listdir(directory):
		entryPath = os.path.join(directory, entry)
		if os.path.isdir(entryPath):
			entrydest = os.path.join(destination, entry)
			if os.path.exists(entrydest):
				if not os.path.isdir(entrydest):
					raise IOError("Failed to copy thee, the destination for the `" + entryPath + "' directory exists and is not a directory")
				copyDirectoryTree(entryPath, entrydest, preserveSymlinks)
			else:
				shutil.copytree(entryPath, entrydest, preserveSymlinks)
		else: #symlinks and files
			if preserveSymlinks:
				shutil.copy(entryPath, directory)
			else:
				shutil.copy(os.path.realpath(entryPath), directory)    

def recursiveCopy(src, dst):
	# remove everything from dest folder
	print ("copying from %s to %s"%(src, dst))        
	rmdir(dst)    
	copyDirectoryTree(src, dst)

def netuse (ip):
	os.system(r"net use \\%s /del 1>nul 2>nul"%ip)
	os.system(r"net use \\%s /user:gml gml 1>nul 2>nul"%ip)	

def upload():
	for ip,config in slaves:		
		print ("provisioning %s with the work package"%(ip))
		netuse(ip)
		remoteDir = r"\\%s\gml"%(ip)		
		recursiveCopy(workPackage, remoteDir)
		print "-"*50

def stopLoopScript():
	for ip,config in slaves:
		print ("-"*100)
		print ("trying to stop loop script for: %s"%(ip))
		os.system(r"pskill \\%s -u gml -p gml python3.exe"%ip)

def startLoopScript():
	for ip,config in slaves:
		print ("trying to start loop script for: %s"%(ip))
		netuse(ip)
		startCommand = r'psexec.exe \\%s -d -u gml -p gml -n 5 -w "\\%s\gml\work" "\\%s\gml\bin\python\python3.exe" "\\%s\gml\bin\python\slave.py"'%(ip,ip,ip,ip)
		print ('issuing: %s'%(startCommand))
		os.system(startCommand)

#-----------------------------------------------------------------------------------------------------------

def getDefault():
	default = {
        "AlgorithmName" : "ProbVectorMachine",
        "KernelType" : "Scalar",
        "KernelParamInt" : 3,
        "KernelParamDouble" : 0.0,
        
        "BlockFileSize" : 1, # in MB
        "BlockFilePrefix" : "pvm",
	    
        #training related vars
        "Lambda" : 1.1,
        "WindowSize" : 10,
	    "NrUpdatesPerLastBlock": 5,
	    "NrUpdatesPerNormBlock": 25,
	    
	    # connectors
	    "Connector" : {
	        "DataFileName":"500.050P001P.compressed.cache.5",
	        "Type":"BitConnector"
	    },  	    
	    "ConnectorTest" : {
	        "DataFileName":"500.050P001P.compressed.cache.5",
	        "Type":"BitConnector"
	    },
	    
	    "Notifier" : {
	        "UseColors": True,
	        "Type": "FileNotifier",
	        "FileName": "notifier.log"
	    },
	    "T" : 1.0
	    # to be put in by the scheduler
	    #Command = "PreCompGramMatrix"
	    #IterationNo = 0
	    #"ThreadsCount":2, "BlockStart":0, "BlockCount": 5, "WindowSize":100

	}
	return default

def makeCmdFile(paramDict):
	cmd = ""
	for key, val in paramDict.items():
		if not isinstance(val, dict):
			if isinstance(val, str):
				cmd += key + " = \"" + str(val) + "\"\n"
			else: 
				cmd += key + " = " + str(val) + "\n"
		else:
			cmd += key + " = {\n"
			for k,v in val.items():
				if isinstance(v, str):
					cmd += "\t" + k + " = \"" + str(v) + "\"\n"
				else: 
					cmd += "\t" + k + " = " + str(v) + "\n"
			cmd += "}\n"
	return cmd

def getCmdString(paramList):
	param = dict()
	for x in paramList:
		for k,v in x.items():
			param[k] = v
	return makeCmdFile(param)

def removeDoneCmds(ip):
	dones = glob.glob(r"\\%s\gml\cmd\*.done"%(ip))
	dones += glob.glob(r"\\%s\gml\cmd\*.cmd"%(ip))
	for d in dones:
		os.unlink(d)

def putCmd (cmd, local=False,itNumber=0, varT=1):	
	print ("putting command: %s for every slave"%(cmd))
	cmdFiles = []
	
	if cmd in ["MergeKprimeFiles"]:
		local = True	

	# if a local cmd only, get config for local ip
	if local:
		for ip, config in slaves:
			if ip == "127.0.0.1":
				ipuple = [("127.0.0.1", config)]
	else:
		ipuple = slaves
		
	# execute command on all slaves
	for ip, config in ipuple:
		print (str("-"*50) + "\nhandling ip: %s"%(ip))
		netuse(ip)
		removeDoneCmds(ip)
		
		if cmd in ["PreCompGramMatrix","MergeKprimeFiles","PreCompEqNorm",
		           "InitStateVars","BlockTraining","LastBlockTraining",
		           "GatherBlockStates","Clasify","BlockScoreComputation"]:
			cmdDict = {"Command":cmd,"IterationNo":itNumber, "T":varT}
		else:
			print ("unrecognized command: %s"%(cmd))
			return
			
		text = getCmdString([ getDefault(), config, cmdDict ])
		dstFileName = getDefault()["BlockFilePrefix"] + "-" +cmd + "-" + ip
		dstFilePath = r"\\%s\gml\cmd\%s"%(ip, dstFileName)			
		dstTmp = dstFilePath + ".tmp"
		
		tmp = open (dstTmp, "wt")
		tmp.write(text)
		tmp.close()
		dstFilePath = dstFilePath + ".cmd"
		shutil.move(dstTmp, dstFilePath)
		cmdFiles.append(dstFilePath)
		print ("cmd file: %s"%(dstFilePath))	

	time2wait = 3600
	
	print ("waiting %d sec for slaves to finish working"%(time2wait))
	timeStart = time.time()
	while len(cmdFiles)>0:
		time.sleep(5)
		print ("-"*50)
		timeNow = time.time()
		if timeNow - timeStart >= time2wait: break
		
		for cmdFile in cmdFiles:
			done = cmdFile + ".done"
			if os.path.exists(done):
				print ("finished: %s"%(done))
				cmdFiles.remove(cmdFile)
				os.unlink(done)
		
		for cmdFile in cmdFiles:
			active = cmdFile+".run"
			if os.path.exists(active):
				print ("active: %s"%(active))
				
		for cmdFile in cmdFiles:			
			if os.path.exists(cmdFile):
				print ("idle: %s"%(cmdFile))
		
def copySlaves2Local(pattern):
	print ("-"*100)
	print ("copying files with pattern : %s from all slaves to local"%(pattern))	
	localIp = "127.0.0.1"
	for ip, config in slaves:
		if ip == localIp: continue
		netuse(ip)
		print "-"*50
		print "copying from ip: %s"%(ip)
		files = glob.glob(r"\\%s\gml\work\%s"%(ip, pattern))
		localBase = r"\\%s\gml\work"%(localIp)
		for f in files:
			localPath = os.path.join(localBase, os.path.basename(f))
			print ("\t%s -> %s"%(f, localPath))
			shutil.copy(f, localPath)

def copyLocal2Slaves(pattern):
	print ("-"*100)
	print ("copying files with pattern : %s to all slaves"%(pattern))
	localIp = "127.0.0.1"
	files = glob.glob(r"\\%s\gml\work\%s"%(localIp, pattern))
	for ip, config in slaves:		
		if ip == localIp: continue
		netuse(ip)
		print ("copying to ip: %s"%(ip))
		for f in files:
			remoteFile = f.replace(localIp,ip)
			try:
				os.unlink(remoteFile)
			except:pass
			shutil.copy(f, remoteFile)
			print ("\t%s -> %s"%(f, remoteFile))
			
def deleteFromSlaves(pattern):	
	print ("-"*100)
	print ("deleting files with pattern : %s from all slaves"%(pattern))		
	for ip, config in slaves:		
		netuse(ip)
		print "-"*50
		print "removing from from ip: %s"%(ip)
		files = glob.glob(r"\\%s\gml\work\%s"%(ip, pattern))		
		for f in files:
			print ("\t%s"%(f))
			os.unlink(f)
#-----------------------------------------------------------------------------------------------------------
def putMergeEqNormFiles():
	copySlaves2Local("*.eqnorm.??")
	putCmd("MergeKprimeFiles",local=True)
	deleteFromSlaves("*.eqnorm.??")
	copyLocal2Slaves("*.eqnorm.all")
#-----------------------------------------------------------------------------------------------------------
def putMergeKPrimeFiles():
	copySlaves2Local("*.kprime.??")
	putCmd("MergeKprimeFiles",local=True)	
	deleteFromSlaves("*.kprime.??")	
	copyLocal2Slaves("*.kprime.all")		
#-----------------------------------------------------------------------------------------------------------
def test():
	#copyPackage()
	
	startLoopScript()
	time.sleep(5)
	stopLoopScript()
#-----------------------------------------------------------------------------------------------------------	
def initial_computation():
	putCmd("PreCompGramMatrix")
	putMergeKPrimeFiles()
	putCmd("PreCompEqNorm")	
	putCmd("InitStateVars")
#-----------------------------------------------------------------------------------------------------------	
def iterate(iterNo, varT):
	putCmd("BlockTraining", False, iterNo, varT)
	putCmd("LastBlockTraining", True, iterNo, varT)
	copySlaves2Local("pvm.state.iter.%04d.block.????"%(iterNo))	
	deleteFromSlaves("pvm.state.iter.%04d.block.all"%(iterNo - 1))
		
	putCmd("GatherBlockStates", True, iterNo, varT)
	
	deleteFromSlaves("pvm.state.iter.%04d.block.????"%(iterNo))		
	files2rem = glob.glob("%s.state.iter.%04d.block.????"%(getDefault()["BlockFilePrefix"],iterNo - 1))
	files2rem += glob.glob("%s.state.iter.%04d.block.???"%(getDefault()["BlockFilePrefix"],iterNo - 1))
	for item in files2rem:			
		try : os.unlink(item)
		except : pass	
			
	copyLocal2Slaves("pvm.state.iter.%04d.block.all"%(iterNo))
#-----------------------------------------------------------------------------------------------------------	
def solve_for_T(varT, nrIterMax, nrIterStable):
	score_vect = []
	
		
	print("Performing training for T = %.05f"%varT)
	
	copyLocal2Slaves("pvm.state.iter.%04d.block.all"%0)
	
	for it in range(1, nrIterMax):
		iterate(it, varT)
		
		score = open("%s.state.iter.%04d.block.score"%(getDefault()["BlockFilePrefix"],it)).read().strip()
		score = float(score)
		score_vect.append(score)
			
		shutil.copy("%s.state.iter.%04d.block.score"%(getDefault()["BlockFilePrefix"],it), "%s.state.score.current"%(getDefault()["BlockFilePrefix"]))
		
		if it > 1:
			try : os.unlink("%s.state.iter.%04d.block.score"%(getDefault()["BlockFilePrefix"],it-1))
			except : pass
			
		if score == 0:
			shutil.copy("%s.state.iter.%04d.block.all"%(getDefault()["BlockFilePrefix"],it), "%s.state.vart.%05.05f.all"%(getDefault()["BlockFilePrefix"],varT))
		elif it > nrIterStable:
			if min(score_vect[0:len(score_vect)-nrIterStable]) - min(score_vect[-nrIterStable:]) < 0.001:
				break
			
	files2rem = glob.glob("%s.state.iter.????.block.all"%(getDefault()["BlockFilePrefix"],it-1))
	
	if min(score_vect[0:len(score_vect)]) < 0.001:
		return True
			
	return False	
#-----------------------------------------------------------------------------------------------------------	
def solve(nrIterMax, nrIterStable):
	varTright = 0.0	
	found_one = False
	for it in range(1, 30):
		varTleft = varTright
		varTright = (varTleft + 1)*2
		
		if (solve_for_T(varTright, 9999, 75)):
			found_one = True
			break
		
	if found_one == False:		
		return False
	
	try : shutil.copy("%s.state.vart.%05.05f.all"%(getDefault()["BlockFilePrefix"],varTright), "%s.state.iter.%04d.block.all"%(getDefault()["BlockFilePrefix"],0))
	except : pass	
	
	while varTright - varTleft > 0.001:
		varTcenter = (varTleft + varTright)/2
		if solve_for_T(varTcenter, 9999, 75):
			varTright = varTcenter			
		else:
			varTleft = varTcenter
			
		try : shutil.copy("%s.state.vart.%05.05f.all"%(getDefault()["BlockFilePrefix"],varTright), "%s.state.iter.%04d.block.all"%(getDefault()["BlockFilePrefix"],0))
		except : pass

#-----------------------------------------------------------------------------------------------------------	
def complete_run():
	upload()
	startLoopScript()
	initial_computation()
	solve(2000, 70)	
	stopLoopScript()
#-----------------------------------------------------------------------------------------------------------		
#test()
#print (makeCmdFile(getDefaultParams()))

#stopLoopScript()
#upload()


#startLoopScript()
#deleteFromSlaves("pvm.kprime.*")
#putMergeKPrimeFiles()
	
#initial_computation()
#putCmd ("PreCompGramMatrix")
#putCmd ("PreCompEqNorm")
#putMergeKPrimeFiles()
#putCmd ("InitStateVars")
#putCmd ("BlockTraining",itNumber=1)
#copyLocal2Slaves("pvm.kernel.??")
solve(2000, 70)
#complete_run()
