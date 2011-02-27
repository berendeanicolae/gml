__author__		="HSC, GDT, MCU"
__date__		="$Feb 27, 2011 03:02:34 PM$"
__version__		="0.1.0.1"
__copyright__	= "Copytight 2011 %s" %__author__
__webpage__		= "http://code.google.com/p/gml/"

REPO = "http://students.info.uaic.ro/~stefan.hanu/gml/repo.lst"

import sys, hashlib, zipfile

if sys.version_info < (2, 6):
	raise("Not tested on this version of python. If you think it might work, please disable this check.")
	from urllib import urlretrieve
elif sys.version_info > (3, 0):
	from urllib.request import urlretrieve
else:
	from urllib import urlretrieve

from optparse import OptionParser, OptionGroup
from time import strftime, localtime
from os.path import basename, dirname, isfile, exists, join
from os import makedirs

LOCAL_REPOSITORY = "repo.lst"

class Utils:
	def md5sum(filePath):
		md5 = hashlib.md5()
		file = open(filePath, "rb")
		while True:
			data = file.read(8192)
			if not data:
				break
			md5.update(data)

		file.close()   
		return md5.hexdigest()

class ConsoleMessages:
	def __source_info(self):
		return sys._getframe(2).f_code.co_filename, sys._getframe(2).f_lineno, sys._getframe(2).f_code.co_name

	def __timestamp(self):
		return strftime("%d %b %Y %H:%M:%S", localtime())

	def print_error(self, msg):
		fname, line, func = self.__source_info()
		print("[ERROR]\n   Message\t: %s\n   Source\t: %s:%d::%s\n   Timestamp\t: %s" %(msg, fname, line, func, self.__timestamp()))
		return False
	
	def print_msg(self, msg):
		print(msg)
		
class unzip:
	cm = None
	def __init__(self, verbose = False, percent = 10):
		self.verbose = verbose
		self.percent = percent
		self.cm = ConsoleMessages()
		
	def __makedirs(self, fname, basedir):
		directories = self.__listdirs(fname)
		for dname in directories:
			curdir = join(basedir, dname).replace("/","")
			if not exists(curdir):
				try:
					makedirs(curdir)
				except:
					return self.cm.print_error("Could not create directory: \"%s\"" %curdir)
		return True

	def __listdirs(self, fname):
		zf = zipfile.ZipFile(fname)
		dirs = []
		for name in zf.namelist():
			if name.endswith('/'):
				dirs.append(name)
		dirs.sort()
		return dirs

	def extract(self, fname, root):
		if not isfile(fname):
			return self.cm.print_error("File doesn't exist: \"%s\"" %fname)
			
		if not root.endswith(':') and not exists(root):
			try:
				makedirs(root)
			except:
				return cm.print_error("Could not make root directory: \"%s\"" %root)
				
		zf = zipfile.ZipFile(fname)
		if zf.testzip() != None:
			return self.cm.print_error("Invalid zip archive: \"%s\"" %fname)
			
		zf.extractall(path=root)
		return True

class Downloader:
	repository = None
	cm = None
	root = ""
	lrepo_file = ""

	def __init__(self, repository=None, root=""):
		self.repository = repository
		self.root = root
		self.lrepo_file = join(self.root, LOCAL_REPOSITORY)
		self.cm = ConsoleMessages()

	def __report_hook(self, numblocks, blocksize, filesize, dest):
		try:
			percent = min((numblocks*blocksize*100)/filesize, 100)
		except:
			percent = 100
		if numblocks != 0:
			sys.stdout.write("\b"*70)
			sys.stdout.write("[%3d%%] %-63s" % (percent, dest))
	
	def update(self):
		self.cm.print_msg("Update started...")
		ret = True
		if self.repository == None:
			return False
		if not self.download(self.repository, self.lrepo_file):
			return False
		if isfile(self.lrepo_file):
			RFILE = None
			lines = []
			try:
				RFILE = open(self.lrepo_file)
				lines = RFILE.readlines()
			except:
				return False
			finally:
				if RFILE != None:
					RFILE.close()
			for line in lines:
				line = line.strip()
				if len(line) == 0 or line.startswith("#"):
					continue
				sline = line.split("|")
				if len(sline) != 4:
					return self.cm.print_error("Invalid line in repo.lst: \"%s\". Update/Install will not continue" %line)
				if sline[0].startswith("action="):
					action = sline[0].split("=")[-1]
					if action == "get":
						fname = join(self.root, sline[2])
						if isfile(fname):
							if sline[3] != Utils.md5sum(fname):
								ret &= self.download(sline[1], fname)
							else:
								self.cm.print_msg("[100%%] Already latest version: %s"  %fname)
						else:
							ret &= self.download(sline[1], fname)
					elif action == "deflate":
						unz = unzip()
						if not unz.extract(join(self.root, sline[1]), join(self.root, sline[2])):
							return False
					else:
						return self.cm.print_error("Unknown action from repository file: \"%s\". Will not continue" %action)
				else:
					return self.cm.print_error("Unknown repository format")
		else:
			ret = False
		return ret
	
	def download(self, src, dest):
		sys.stdout.write("Download: " + src)
		if not exists(dirname(dest)):
			try:
				makedirs(dirname(dest))
			except:
				return self.cm.print_error("Could not create target dir: \"%s\"" %dirname(dest))
		try:
			if sys.stdout.isatty():
				urlretrieve(src, dest, lambda nb, bs, fs, dst=dest: self.__report_hook(nb, bs, fs, dst))
			else:
				urlretrieve(src, dest)
		except:
			sys.stdout.write(" [ERROR]\n")
			return False
		else:
			sys.stdout.write(" [OK]\n")
		return True

#upgrade, version, status, install, create-project, etc
def get_cmd_options():
	parser = OptionParser(version=__version__)

	pkg_group = OptionGroup(
			parser,
			"Installation options",
			"These options control install and update related actions"
			)
	pkg_group.add_option(
			"-i", "--install",
			action = "store",
			dest   = "install_path",
			default= None,
			help   = "Download and install GML to given path. An example would be: \"gml.py -i C:\\GML\""
			)

	parser.add_option_group(pkg_group)

	return parser.parse_args()

if __name__ == '__main__':
	cm = ConsoleMessages()
	
	if len(sys.argv) == 1:
		cm.print_msg("Nothing to do. Use -h or --help for more information")

	(options, args) = get_cmd_options()

	if not (options.install_path is None) and (len(options.install_path) > 0):
		if exists(options.install_path):
			if isfile(options.install_path):
				cm.print_error("Install destination exists and is a file. Will not continue")
				exit(1)
		else:
			try:
				makedirs(options.install_path)
			except:
				cm.print_error("Could not create %s. Please check if current user has enough rights" %options.install_path)
				exit(1)
		d = Downloader(repository=REPO, root=options.install_path)
		if not d.update():
			cm.print_error("Could not update GML")
			exit(1)
		else:
			cm.print_msg("== Done ==")