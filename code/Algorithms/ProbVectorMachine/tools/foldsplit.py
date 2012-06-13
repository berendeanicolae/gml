import gmlpy, sys

def generateDbFraction(dbInput, dbOutput, procStart, procCount):	
	d = {
	    "AlgorithmName":"CacheBuilder",
	    "CacheName":dbOutput,
	    "Command":"CreateCache",
	    "Connector":{
	        "Type":"TypeFloatConnector",
	        "StoreFeatureName":False,
	        "StoreRecordHash":False,          		        
	        "Connector": {        
	            "Type":"SplitConnector",
	            "StoreFeatureName":False,
	            "StoreRecordHash":False,
	            "SplitMode":"UniformPercentage",
	            "Start":procStart,
	            "End":(procStart+procCount)%100,        
	            "Connector" : {
	                "Type": "TypeFloatConnector",                
	                "DataFileName": dbInput,
	                "StoreFeatureName":False,
	                "StoreRecordHash":False,
	            },        
	        },	    
	    },
	    "Notifier":{
	        "Type":"FileNotifier",
	        "FileName": "notifier.log",
	        "UseColors":False
	    }		
	}	
	gmlpy.Run(d)
	
def shuffleDb(dbInput, dbOutput):
	d = {
	    "AlgorithmName":"CacheBuilder",
	    "CacheName":dbOutput,
	    "Command":"CreateCache",
	    "Connector":{
	        "Type":"TypeFloatConnector",
	        "StoreFeatureName":False,
	        "StoreRecordHash":False,          		        
	        "Connector" : {	    
	            "Type" : "ShuffleConnector",
	            "Method" : "Random",	        
	            "StoreFeatureName":False,
	            "StoreRecordHash":False,    	    	            		            	        
	            "Connector" : {
	                "Type": "FullCacheConnector",                
	                "DataFileName": dbInput,
	                "StoreFeatureName":False,
	                "StoreRecordHash":False,
	            },        
	        },
	    },
	    "Notifier":{
	        "Type":"FileNotifier",
	        "FileName": "notifier.log",
	        "UseColors":False
	    }		
	}	
	gmlpy.Run(d)	
	
def bitConnector2FullCacheConnector (dbInput, dbOutput):	
	d = {
        "AlgorithmName":"CacheBuilder",
        "CacheName":dbOutput,
        "Command":"CreateCache",
        "Connector":{
            "Type":"TypeFloatConnector",
            "StoreFeatureName":False,
            "StoreRecordHash":False,          		        
            "Connector" : {	    
                "Type" : "ShuffleConnector",
                "Method" : "Random",	        
                "StoreFeatureName":False,
                "StoreRecordHash":False,    	    	            		            	        
                "Connector" : {
                    "Type": "BitConnector",                
                    "DataFileName": dbInput,
                    "StoreFeatureName":False,
                    "StoreRecordHash":False,
                },        
            },
        },
        "Notifier":{
            "Type":"FileNotifier",
            "FileName": "notifier.log",
            "UseColors":False
        }		
    }	
	gmlpy.Run(d)	
	
def splitDatabaseInFolds (database, nrOfFolds):	

	shuffledDatabase = database + ".shuffle"
	shuffleDb(database, shuffledDatabase)
	
	splitSize = 100/nrOfFolds
	for x in range(0,nrOfFolds):
		procStart1 = splitSize*x
		procCount1 = splitSize
		procStart2 = (splitSize + procStart1)%100
		procCount2 = 100-splitSize
		generateDbFraction(shuffledDatabase, "%s-%dof%d-test.cache"%(database, x+1, nrOfFolds), procStart1, procCount1)
		generateDbFraction(shuffledDatabase, "%s-%dof%d-train.cache"%(database, x+1, nrOfFolds), procStart2, procCount2)
		
		
	
def main ():
	if len(sys.argv) < 2:
		print ("usage: foldsplit.py DatabaseFileName")
		return
	database = sys.argv[1]
	splitDatabaseInFolds(database, 5)

bitConnector2FullCacheConnector(sys.argv[1], sys.argv[2])	
main()