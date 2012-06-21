import os, gmlpy, math, glob, shutil
global trainDataSet, testDataSet
global connectorType
def getParameterConfig():
    param = {
        "AlgorithmName" : "ProbVectorMachine",
        "KernelType" : "Scalar",
        "KernelParamInt" : 3,
        "KernelParamDouble" : 0.0,
        
        "BlockFileSize" : 1024,
        "BlockStart" : 0,
        "BlockCount" : 1,
        "BlockFilePrefix" : "pvm",
        
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
        "Type":"ConsoleNotifier"
        }
    return notif

def ExecuteCommand(cmd, itNumber=0): 
    line50 = '-'*50
    print ("%s%s%s"%(line50, cmd, line50))
    mydict = getParameterConfig()
    con = getConnectorDict()
    testCon = getConnectorTestDict()
    notif = getNotifierDict()
    
    mydict["Connector"] = con
    mydict["Notifier"] = notif
    mydict["ConnectorTest"] = testCon
    
    mydict["Command"] = cmd
    mydict["IterationNo"] = itNumber
    
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
    print ("%s %s it: %d %s"%(line50, mydict["Command"], itNumber, line50))        
    gmlpy.Run(mydict)     
    
    mydict["Command"] = "LastBlockTraining"
    line50 = '-'*50
    print ("%s %s it: %d %s"%(line50, mydict["Command"], itNumber, line50))        
    gmlpy.Run(mydict)     
    
    
def IterateTraining (nrIter, varT, nrIterStable):
    paramDict = getParameterConfig()
    score_vect = []

    print ("performing training for T = %.05f"%varT)
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
        
        shutil.copy("%s.state.iter.%04d.block.score"%(paramDict["BlockFilePrefix"],it), "%s.state.score.current"%(paramDict["BlockFilePrefix"]))        
        
        # remove garbage
        if it>1:
            files2rem = glob.glob("%s.state.iter.%04d.block.????"%(paramDict["BlockFilePrefix"],it-1))
            files2rem += glob.glob("%s.state.iter.%04d.block.last"%(paramDict["BlockFilePrefix"],it-1))
            files2rem += glob.glob("%s.state.iter.%04d.block.score"%(paramDict["BlockFilePrefix"],it-1))
            files2rem += glob.glob("%s.state.iter.%04d.block.all"%(paramDict["BlockFilePrefix"],it-1))
            for item in files2rem:
                print ("removing file: %s"%item)
                try:os.unlink(item)#pass#
                except: pass       
                
        
        
        if score == 0:                  
            shutil.copy("%s.state.iter.%04d.block.all"%(paramDict["BlockFilePrefix"],it), "%s.state.vart.%05.05f.all"%(paramDict["BlockFilePrefix"],varT))
            print ("yyeeeeeyyyy")
            retValue = True
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
        print ("removing file: %s"%item)
        try:os.unlink(item)#pass#
        except: pass       
        
    return retValue
    

def BinarySearchTraining ():
    varTLeft  = 0.0
    varTRight = 0.0
    epsilon = 0.001
    maxNrIter = 4000
    nrIterStable = 50
    
    foundFeasible = False
    
    for pow2 in range (0, 10):
        varTLeft = varTRight
        varTRight = 2**pow2 + 1
        if IterateTraining(maxNrIter, varTRight, nrIterStable):
            foundFeasible = True            
            break
    if not foundFeasible:
        print ("cannot perform training on this database. sorry")
        exit(1)
        
    nrIterStable *= 3
    
    while varTRight-varTLeft > epsilon:
        varTCenter = (varTLeft + varTRight)/2
        if IterateTraining(maxNrIter, varTCenter, nrIterStable):
            varTRight = varTCenter
        else:
            varTLeft = varTCenter
    print ("found T = %.05f"%varTRight)
    

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
    
if __name__ == "__main__":
    trainDataSet = "breast-cancer-wisconsin.data.fullcache"
    testDataSet = "breast-cancer-wisconsin.data.fullcache"
    connectorType = "FullCacheConnector"
    MakeInitialPrep()    
    BinarySearchTraining()

    PerformClassification()
    #IterateTraining(4000, 1.0, 100);