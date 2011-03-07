#!/usr/bin/python
# -*- coding: utf-8 -*-
__author__		="HSC, GDT, MCU"
__date__		="$Feb 27, 2011 03:02:34 PM$"
__version__		="0.1.0.1"
__copyright__		= "Copytight 2011 %s" %__author__
__webpage__		= "http://code.google.com/p/gml/"

REPO = "http://www.infoiasi.ro/gml/repo.lst"
LOCAL_REPOSITORY = "repo.lst"

# === DO NOT MODIFY BELOW ===

import sys, os, re, hashlib, zipfile

if sys.version_info < (2, 6):
	raise("Not tested on this version of python. If you think it might work, please disable this check.")
	from urllib import urlretrieve
elif sys.version_info > (3, 0):
	from urllib.request import urlretrieve
else:
	from urllib import urlretrieve

from ctypes import *
from glob import glob
from time import strftime, localtime, sleep
from os.path import basename, dirname, isfile, exists, join
from os import makedirs
from os import name as OS_TYPE

plugin_data = {"algorithms":"alg", "connectors":"dbc", "databases":"db", "notifiers":"ntf"}

class OParse:
	__options = {}

	def __init__(self, argv, version):
		self.argv = argv
		self.version = version

		self.add_option('version', short_alias='-v', long_alias='--version', param_nr=0, help='Display version information and exit', callback=self.display_version)
		self.add_option('help', short_alias='-h', long_alias='--help', param_nr=0, help='Display commands and a description', callback=self.usage)

	def __is_short_opt(self, opt):
		if not opt.startswith("-") or len(opt) != 2:
			return ""
		for _opt in self.__options.keys():
			if self.__options[_opt].short_alias == opt.lower():
				return self.__options[_opt].__name__

	def __is_long_opt(self, opt):
		if not opt.startswith("--") or len(opt) < 4:
			return ""
		for _opt in self.__options.keys():
			if self.__options[_opt].long_alias == opt.lower():
				return self.__options[_opt].__name__

	def display_version(self, params):
		print("Version: %s" %self.version)
	
	def usage(self, params):
		from os.path import basename
		print("Usage: %s command [options]\n\nCommands:" %basename(self.argv[0]))
		options = [x for x in self.__options.keys()]
		options.sort()
		for opt in options:
			tmp_params = "0"
			if self.__options[opt].param_nr < 0:
				tmp_params = "variable number"
			else:
				if self.__options[opt].param_nr == 0:
					tmp_params = "None"
				else:
					tmp_params = str(self.__options[opt].param_nr)
			if self.__options[opt].accept_no_param:
				tmp_params += " (use with 0 parameters for more info)"
			print("  %s, %s, %s\n\tdescription: %s\n\tparameters: %s" %(
				self.__options[opt].__name__,
				self.__options[opt].short_alias,
				self.__options[opt].long_alias,
				self.__options[opt].help,
				tmp_params)
			)

	def add_option(self, name, short_alias="", long_alias="", param_nr=0, accept_no_param=False, help="", callback=None):
		name = name.lower()
		short_alias = short_alias.lower()
		long_alias = long_alias.lower()
		if name in self.__options.keys():
			raise NameError("Duplicate option: %s" %name)
		for argv in self.__options.keys():
			if self.__options[argv].short_alias == short_alias:
				raise NameError("Duplicate short alias: %s. Already used for '%s' option." %(short_alias, argv))
			if self.__options[argv].long_alias == long_alias:
				raise NameError("Duplicate long alias: %s. Already used for '%s' option." %(long_alias, argv))
		tmp_obj = type(
				name,
				(object,),
				{
					'callback':callback,
					'help':help,
					'short_alias':short_alias.lower(),
					'long_alias':long_alias.lower(),
					'param_nr':param_nr,
					'accept_no_param':accept_no_param
				}
			      )
		self.__options[name] = tmp_obj
	
	def parse(self):
		if len(sys.argv) == 1:
			self.usage([])
			return
		consumed = []
		for argv in self.argv[1:]:
			if argv in consumed:
				continue
			tmp = ""
			if not argv in self.__options.keys():
				tmp = self.__is_short_opt(argv)
				if tmp == "":
					tmp = self.__is_long_opt(argv)
				if tmp == "" and not tmp is None:
					print("Unknown option: %s" %argv)
					return
				else:
					argv = tmp
			opt = self.__options[argv]
			if opt.param_nr == -1:
				opt.param_nr = len(self.argv) - 2
			if len(self.argv) < 2 + opt.param_nr and not opt.accept_no_param: #FIXME
				print("[ERROR] Expecting %d parameters" %opt.param_nr)
				return
			if not opt.callback is None:
				opt.callback(self.argv[2:2+opt.param_nr])
			else:
				print("[WARN] No callback function for '%s' option set!" %argv)
			consumed += (self.argv[:2+opt.param_nr])
			break #FIXME: allows only one command at a time
		if len(consumed) != len(self.argv):
			print("Some options did not get parsed") #TODO: add what options
			
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
		
def __add_to_path(path):
	if OS_TYPE == 'nt':
		from winreg import CreateKey, SetValueEx, QueryValueEx, HKEY_CURRENT_USER, REG_EXPAND_SZ
		from win32gui import SendMessage
		from win32con import HWND_BROADCAST, WM_SETTINGCHANGE
		envpath = None
		HKCU = HKEY_CURRENT_USER
		ENV = "Environment"
		PATH = "PATH"
		DEFAULT = "%PATH%"
		with CreateKey(HKCU, ENV) as key:
			try:
				envpath = QueryValueEx(key, PATH)[0]
			except WindowsError:
				envpath = DEFAULT
			paths = [envpath]
			if path and path not in envpath and os.path.isdir(path):
					paths.append(path)
			envpath = os.pathsep.join(paths)
			try:
				SetValueEx(key, PATH, 0, REG_EXPAND_SZ, envpath)
				SendMessage(HWND_BROADCAST, WM_SETTINGCHANGE, 0, 'Environment')
			except WindowsError:
				return False
			return True
	else:
		return False
		
def __add_to_pypath(path):
	if OS_TYPE == 'nt':
		from winreg import CreateKey, SetValueEx, QueryValueEx, HKEY_LOCAL_MACHINE, REG_SZ
		envpath = ""
		HKLM = HKEY_LOCAL_MACHINE
		PythonPath = r"SOFTWARE\Python\PythonCore\%s\PythonPath" %sys.version.split()[0]
		with CreateKey(HKLM, PythonPath) as key:
			try:
				envpath = QueryValueEx(key, "")[0]
			except WindowsError:
				return False
			paths = [envpath]
			if path and path not in envpath and os.path.isdir(path):
					paths.append(path)
			envpath = os.pathsep.join(paths)
			try:
				SetValueEx(key, "", 0, REG_SZ, envpath)
			except WindowsError:
				return False
			return True
	else:
		return False
		
def add_to_path(path):
	cm = ConsoleMessages()
	cm.print_msg("\n[Registry operations]")
	if not __add_to_path(path):
		cm.print_error("Could not add GML root dir to PATH")
		exit(1)
	else:
		cm.print_msg("  [ -- ] Done adding GML root dir to PATH. Changes will be\n  visible in a new command line")
	#if not __add_to_pypath(path):
	#	cm.print_error("Could not add GML root dir to PYTHONPATH")
	#	exit(1)
	#else:
	#	cm.print_msg("  [ -- ] Done adding GML root dir to PYTHONPATH")
		
class Plugin:
	__no_data = b"No data set"
	
	def __init__(self, plg_name):
		self.plg_name = plg_name
		self.__cm = ConsoleMessages()
		try:
			self.dll = CDLL(self.plg_name)
		except:
			self.__cm.print_error("Could not load plugin: %s" %self.plg_name)
	
	def get_author(self):
		author = c_char_p(self.__no_data)
		try:
			author = c_char_p(self.dll.GetInterfaceAuthor())
		except:
			pass
		if author.value is None or author.value == "":
			return self.__no_data.decode('ascii')
		else:
			return author.value.decode('ascii')
			
	def get_description(self):
		description = c_char_p(self.__no_data)
		try:
			description = c_char_p(self.dll.GetInterfaceDescription())
		except:
			pass
		if description.value is None or description.value == "":
			return self.__no_data.decode('ascii')
		else:
			return description.value.decode('ascii')
			
	def get_version(self, ret_type=str):
		version = c_int(0)
		try:
			version = c_int(self.dll.GetInterfaceVersion())
		except:
			pass
		if version.value is None or version.value == 0:
			return self.__no_data
		else:
			return version.value
			
	def get_properties(self):
		tmp_fname = b"~prop.tmp"
		ret = "\n"
		ready = c_bool(False)
		try:
			ready = c_bool(self.dll.SaveInterfaceTemplate(c_char_p(tmp_fname)))
		except:
			pass
		if ready:
			RFILE = None
			lines = []
			try:
				RFILE = open(tmp_fname, "rt")
				lines = RFILE.readlines()
			except:
				pass
			finally:
				if RFILE != None:
					RFILE.close()
				if os.path.isfile(tmp_fname):
					try:
						os.remove(tmp_fname)
					except:
						pass
			ignore = False
			for line in lines:
				line = line.strip();
				if len(line) == 0:
					continue
				if line.startswith("["):
					ignore = True
				if line.endswith("]"):
					ignore = False
					continue
				if ignore:
					continue
				ret += "\t" + line + "\n"
		return ret
			

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
			sys.stdout.write("  [%3d%%] %-61s" % (percent, dest))
	
	def update(self):
		self.cm.print_msg("[Update process]")
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
							if sline[3] != md5sum(fname):
								ret &= self.download(sline[1], fname)
							else:
								sys.stdout.write("  [ -- ] %-61s [OK]\n" %fname)
						else:
							ret &= self.download(sline[1], fname)
					elif action == "inflate":
						unz = unzip()
						sys.stdout.write("Inflating %s ..." %sline[1])
						if not unz.extract(join(self.root, sline[1]), join(self.root, sline[2])):
							return False
						sys.stdout.write("\b"*70)
						sys.stdout.write("  [ -- ] Inflating %-51s [OK]\n" %fname)
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

class ActionHandler:
	def __init__(self, root):
		self.__cm = ConsoleMessages()
		self.__root = os.path.dirname(root)

	def handle_install(self, params):
		if os.path.exists(params[0]):
			if os.path.isfile(params[0]):
				self.__cm.print_error("Installation directory exists and it is a file. Install aborted")
				return False
		else:
			try:
				os.makedirs(params[0])
			except:
				self.__cm.print_error("Could not create %s. Please check is user has enough rights" %params[0])
				return False
		d = Downloader(repository=REPO, root=params[0])
		if not d.update():
			self.__cm.print_error("Could not update GML")
			return False
		add_to_path(params[0])
		self.__cm.print_msg("== Done ==")
		
	def handle_list(self, params, show_info=False):
		if len(params) == 0:
			self.__cm.print_msg("Posible parameters:")
			for elem in plugin_data.keys():
				self.__cm.print_msg("  %s" %elem)
		else:
			param = params[0].lower()
			err = True
			for lparam in [x.lower() for x in plugin_data.keys()]: #FIXME: beautifie code
				for found in re.findall(param, lparam):
					found = found.lower()
					if found in [x.lower() for x in plugin_data.keys()]:
						err = False
						for fname in glob(os.path.join(os.path.join(self.__root, found), "*." + plugin_data[found])):
							self.__cm.print_msg("%s" %os.path.basename(fname).split(".")[0])
							if show_info:
								plugin = Plugin(fname)
								self.__cm.print_msg("  Author: %s\n  Version: %d\n  Description: %s\n  Properties: %s\n" %(plugin.get_author(), plugin.get_version(), plugin.get_description(), plugin.get_properties()))
			if err:
				self.__cm.print_msg("Unknown parameter for 'list' command: %s" %param)
				return
		
	def handle_info(self, params):
		if len(params) < 2:
			self.handle_list(params, show_info=True)

if __name__ == '__main__':
	action_handler = ActionHandler(sys.argv[0])
	optparse = OParse(sys.argv, "0.1.0.1")
	optparse.add_option('info', 
				short_alias='-i',  
				long_alias='--info', 
				param_nr=-1, 
				accept_no_param=True, 
				help='Display a description for the given plugins', 
				callback=action_handler.handle_info)
	optparse.add_option('list', 
				short_alias='-l', 
				long_alias='--list', 
				param_nr=1, 
				accept_no_param=True, 
				help='List available options for given pattern', 
				callback=action_handler.handle_list)
	optparse.add_option('install', 
				long_alias='--install', 
				param_nr=1, 
				help='Install GML', 
				callback=action_handler.handle_install)
	optparse.parse()
