import os, shutil, sys, glob, time, subprocess

# \\localhost\gml\bin - where gml.exe resides
# \\localhost\gml\cmd - where the command files are
# \\localhost\gml\work - where the working directory is

workDir = r"\\localhost\gml\work"
nrLoops = 3600

def shouldTerminate():
    global nrLoops
    if nrLoops <= 0: return True
    nrLoops -= 1
    return False

def forceUnlink(file2del):
    try:
        os.unlink(file2del)
    except Exception as exc:
        sys.stderr.write("error deleting file%s exception: %s"%(file2del, repr(exc)))        
        
def out(msg):
    out = open(r"\\localhost\gml\work\slave.out","a")
    out.write(msg+"\n")
    
def err(msg):
    out = open(r"\\localhost\gml\work\slave.err","a")
    out.write(msg+"\n")

def getw(path):
    return os.path.join(workDir, path)

def loop():
    os.chdir(r"\\localhost\gml\work")  
    out("changed work dir to: %s"%(os.getcwd())) 
    
    while not shouldTerminate():
        # look for some command to run and execute it
        cmds = sorted(glob.glob(r"\\localhost\gml\cmd\*.cmd"))        
    
        for cmd in cmds:
            out("found: %s"%(cmd))
            # rename the command file in name.run
            run = cmd+".run"
            forceUnlink(run)
            shutil.move(cmd, run)
            work = getw("current.command.cmd")
            forceUnlink(work)
            out("copying %s to %s"%(run, work))
            shutil.copy(run, work)
            # run the command with gml                        
            stdout, stderr=subprocess.Popen([r'\\localhost\gml\bin\gml.exe', 'run', run ], stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
            out(str(stdout))
            err(str(stderr))
            # rename the command file to .done
            done = cmd+".done"
            forceUnlink(done)
            forceUnlink("current.template.cmd")
            shutil.move(run, done)
            # sleeping for 1 second
        time.sleep(1)        
      
try:  
    out("i am alive and kicking")    
    loop()
    out("exiting for some reason")
except Exception as exc:
    out(repr(exc))