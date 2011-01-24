__author__		="HSC, GDT, MCU"
__date__		="$Jan 23, 2011 11:55:34 PM$"
__version__		="0.1.0.1"
__copyright__	= "Copytight 2011 %s" %__author__
__webpage__		= "http://code.google.com/p/gml/"

from optparse import OptionParser, OptionGroup
from time import strftime, localtime
from urllib import urlretrieve
from inspect import currentframe
from os.path import basename, isfile

import sys

LOCAL_REPOSITORY = "repo.lst"

class ConsoleMessages:
	def __line_number(self):
		return currentframe().f_back.f_lineno

	def __timestamp(self):
		return strftime("%d %b %Y %H:%M:%S", localtime())

	def print_error(self, msg):
		print("[ERROR]\n   Message\t: %s\n   Line\t\t: %d\n   Timestamp\t: %s" %(msg, self.__line_number(), self.__timestamp()))
	
	def print_msg(self, msg):
		print(msg)

class Downloader:
	repository = None
	cm = None

	def __init__(self, repository=None):
		self.repository = repository
		self.cm = ConsoleMessages()

	def __report_hook(self, numblocks, blocksize, filesize, dest):
		try:
			percent = min((numblocks*blocksize*100)/filesize, 100)
		except:
			percent = 100
		if numblocks != 0:
			sys.stdout.write("\b"*70)
			#sys.stdout.write("%-66s%3d%%" % (base, percent))
			sys.stdout.write("[%3d%%] %-63s" % (percent, dest))
	
	def update(self):
		ret = True
		if self.repository == None:
			return False
		if not self.download(self.repository, LOCAL_REPOSITORY):
			return False
		if isfile(LOCAL_REPOSITORY):
			RFILE = None
			lines = []
			try:
				RFILE = open(LOCAL_REPOSITORY)
				lines = RFILE.readlines()
			except:
				return False
			finally:
				if RFILE <> None:
					RFILE.close()
			for line in lines:
				sline = line.rstrip().split("|")
				ret &= self.download(sline[1], sline[2])
		else:
			ret = False
		return ret
	
	def download(self, src, dest):
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
			help   = "Download and install GML to given path. Example: gml.py -i C:\\GML"
			)

	parser.add_option_group(pkg_group)

	return parser.parse_args()

if __name__ == '__main__':
	cm = ConsoleMessages()

	(options, args) = get_cmd_options()

	if not (options.install_path is None) and (len(options.install_path) > 0):
		if exists(options.install_path):
			if isfile(options.install_path):
				cm.print_error("Install destination exists and is a file. Will not continue")
				exit(1)

	d = Downloader("http://students.info.uaic.ro/~stefan.hanu/files.txt")
	if not d.update():
		cm.print_error("Could not update GML")