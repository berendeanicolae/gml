import os, gmlpy, math, glob, shutil

def getParameterConfig():
    param = {
        "AlgorithmName" : "ProbVectorMachine",
        "KernelType" : "Rbf",
        "KernelParamInt" : 0,
        "KernelParamDouble" : 0.125,
        
        "BlockFileSize" : 1,
        "BlockStart" : 0,
        "BlockCount" : 5,
        "BlockFilePrefix" : "bitdefender",
        
        "ThreadsCount" : 2,
        
        #training related vars
        "Lambda" : 1,
        "WindowSize" : 4,       
        }
    return param

def getConnectorDict ():
    con = {
        "DataFileName":"500.050P001P.compressed.cache.5",
        "Type":"BitConnector"
        }
    return con

def getConnectorTestDict ():
    con = {
        "DataFileName":"500.050P001P.compressed.cache.test",
        "Type":"BitConnector"
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
    
    for it in range (1, nrIter):
        PerformTrain4Iter(it, varT)    
        ExecuteCommand("GatherBlockStates",it)
        
        score = open("%s.state.iter.%03d.block.score"%(paramDict["BlockFilePrefix"],it)).read().strip()
        score = float(score)
        score_vect.append(score)
        
        shutil.copy("%s.state.iter.%03d.block.score"%(paramDict["BlockFilePrefix"],it), "%s.state.score.current"%(paramDict["BlockFilePrefix"]))
        
        # remove garbage
        if it>1:
            files2rem = glob.glob("%s.state.iter.%03d.block.???"%(paramDict["BlockFilePrefix"],it-1))
            files2rem += glob.glob("%s.state.iter.%03d.block.last"%(paramDict["BlockFilePrefix"],it-1))
            files2rem += glob.glob("%s.state.iter.%03d.block.score"%(paramDict["BlockFilePrefix"],it-1))
            for item in files2rem:
                print ("removing file: %s"%item)
                try:os.unlink(item)#pass#
                except: pass       
                
        
        
        if score == 0:                  
            shutil.copy("%s.state.iter.%03d.block.all"%(paramDict["BlockFilePrefix"],it), "%s.state.vart.%05.05f.all"%(paramDict["BlockFilePrefix"],varT))
            print ("yyeeeeeyyyy")
            return True
        elif it > nrIterStable:
            if min(score_vect[0:len(score_vect)-nrIterStable]) - min(score_vect[-nrIterStable:]) < 0.001:
                return False
    return False
    

def BinarySearchTraining ():
    varTLeft  = 0.0
    varTRight = 0.0
    epsilon = 0.001
    maxNrIter = 1000
    nrIterStable = 50
    
    foundFeasible = False
    
    for pow2 in range (0, 10):
        varTLeft = varTRight
        varTRight = 2**pow2
        if IterateTraining(maxNrIter, varTRight, nrIterStable):
            foundFeasible = True
            exit(1)
            break
    if not foundFeasible:
        print ("cannot perform training on this database. sorry")
        exit(1)
    
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
    mydict["ModelFile"] = "bitdefender.state.vart.00000.all"
    
    gmlpy.Run(mydict)
    
if __name__ == "__main__":
    MakeInitialPrep()    
    #BinarySearchTraining()
    #PerformClassification()
    IterateTraining(500, 1.0, 50);