import os, gmlpy

def getParameterConfig():
    param = {
        "AlgorithmName" : "ProbVectorMachine",
        "KernelType" : "Scalar",
        "KernelParamInt" : 3,
        "KernelParamDouble" : 0.0,
        
        "BlockFileSize" : 1,
        "BlockStart" : 0,
        "BlockCount" : 5,
        "BlockFilePrefix" : "bitdefender",
        
        "ThreadsCount" : 1,
        
        #training related vars
        "Lambda" : 1,
        "WindowSize" : 10,       
        }
    return param

def getConnectorDict ():
    con = {
        "DataFileName":"500.050P001P.compressed.cache.5",
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
    notif = getNotifierDict()
    
    mydict["Connector"] = con
    mydict["Notifier"] = notif
    
    mydict["Command"] = cmd
    mydict["IterationNo"] = itNumber
    
    gmlpy.Run(mydict)    
    
def MakeInitialPrep():    
    ExecuteCommand("PreCompGramMatrix")
    ExecuteCommand("MergeKPrimeFiles")
    ExecuteCommand("PreCompEqNorm")
    ExecuteCommand("InitStateVars")    
    
def IterationTrain (itNumber):   
    
    mydict = getParameterConfig()
    con = getConnectorDict()
    notif = getNotifierDict()
    
    mydict["Connector"] = con
    mydict["Notifier"] = notif
    
    mydict["IterationNo"] = itNumber
    mydict["T"] = 10.0
    
    mydict["Command"] = "BlockTraining"
    line50 = '-'*50
    print ("%s %s it: %d %s"%(line50, mydict["Command"], itNumber, line50))        
    gmlpy.Run(mydict)     
    
    mydict["Command"] = "LastBlockTraining"
    line50 = '-'*50
    print ("%s %s it: %d %s"%(line50, mydict["Command"], itNumber, line50))        
    gmlpy.Run(mydict)     
    
    
if __name__ == "__main__":
    #MakeInitialPrep()
    score_vect = []
    for it in range (1, 20):
        IterationTrain(it)    
        ExecuteCommand("GatherBlockStates",it)
        
        score = open("bitdefender.state.iter.%03d.block.score"%(it)).read().strip()
        score = float(score)
        score_vect.append(score)
        
        if score == 0: 
            print ("yyeeeeeyyyy")
            break        
            
