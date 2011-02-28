# This file was automatically generated by SWIG (http://www.swig.org).
# Version 2.0.1
#
# Do not make changes to this file unless you know what you are doing--modify
# the SWIG interface file instead.
# This file is compatible with both classic and new-style classes.

from sys import version_info
if version_info >= (2,6,0):
    def swig_import_helper():
        from os.path import dirname
        import imp
        fp = None
        try:
            fp, pathname, description = imp.find_module('_ialgorithm', [dirname(__file__)])
        except ImportError:
            import _ialgorithm
            return _ialgorithm
        if fp is not None:
            try:
                _mod = imp.load_module('_ialgorithm', fp, pathname, description)
            finally:
                fp.close()
            return _mod
    _ialgorithm = swig_import_helper()
    del swig_import_helper
else:
    import _ialgorithm
del version_info
try:
    _swig_property = property
except NameError:
    pass # Python < 2.2 doesn't have 'property'.
def _swig_setattr_nondynamic(self,class_type,name,value,static=1):
    if (name == "thisown"): return self.this.own(value)
    if (name == "this"):
        if type(value).__name__ == 'SwigPyObject':
            self.__dict__[name] = value
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    if (not static) or hasattr(self,name):
        self.__dict__[name] = value
    else:
        raise AttributeError("You cannot add attributes to %s" % self)

def _swig_setattr(self,class_type,name,value):
    return _swig_setattr_nondynamic(self,class_type,name,value,0)

def _swig_getattr(self,class_type,name):
    if (name == "thisown"): return self.this.own()
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError(name)

def _swig_repr(self):
    try: strthis = "proxy of " + self.this.__repr__()
    except: strthis = ""
    return "<%s.%s; %s >" % (self.__class__.__module__, self.__class__.__name__, strthis,)

try:
    _object = object
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0


class SwigPyIterator(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, SwigPyIterator, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, SwigPyIterator, name)
    def __init__(self, *args, **kwargs): raise AttributeError("No constructor defined - class is abstract")
    __repr__ = _swig_repr
    __swig_destroy__ = _ialgorithm.delete_SwigPyIterator
    __del__ = lambda self : None;
    def value(self): return _ialgorithm.SwigPyIterator_value(self)
    def incr(self, n = 1): return _ialgorithm.SwigPyIterator_incr(self, n)
    def decr(self, n = 1): return _ialgorithm.SwigPyIterator_decr(self, n)
    def distance(self, *args): return _ialgorithm.SwigPyIterator_distance(self, *args)
    def equal(self, *args): return _ialgorithm.SwigPyIterator_equal(self, *args)
    def copy(self): return _ialgorithm.SwigPyIterator_copy(self)
    def next(self): return _ialgorithm.SwigPyIterator_next(self)
    def __next__(self): return _ialgorithm.SwigPyIterator___next__(self)
    def previous(self): return _ialgorithm.SwigPyIterator_previous(self)
    def advance(self, *args): return _ialgorithm.SwigPyIterator_advance(self, *args)
    def __eq__(self, *args): return _ialgorithm.SwigPyIterator___eq__(self, *args)
    def __ne__(self, *args): return _ialgorithm.SwigPyIterator___ne__(self, *args)
    def __iadd__(self, *args): return _ialgorithm.SwigPyIterator___iadd__(self, *args)
    def __isub__(self, *args): return _ialgorithm.SwigPyIterator___isub__(self, *args)
    def __add__(self, *args): return _ialgorithm.SwigPyIterator___add__(self, *args)
    def __sub__(self, *args): return _ialgorithm.SwigPyIterator___sub__(self, *args)
    def __iter__(self): return self
SwigPyIterator_swigregister = _ialgorithm.SwigPyIterator_swigregister
SwigPyIterator_swigregister(SwigPyIterator)

class pymap(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pymap, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pymap, name)
    __repr__ = _swig_repr
    def iterator(self): return _ialgorithm.pymap_iterator(self)
    def __iter__(self): return self.iterator()
    def __nonzero__(self): return _ialgorithm.pymap___nonzero__(self)
    def __bool__(self): return _ialgorithm.pymap___bool__(self)
    def __len__(self): return _ialgorithm.pymap___len__(self)
    def __getitem__(self, *args): return _ialgorithm.pymap___getitem__(self, *args)
    def __delitem__(self, *args): return _ialgorithm.pymap___delitem__(self, *args)
    def has_key(self, *args): return _ialgorithm.pymap_has_key(self, *args)
    def keys(self): return _ialgorithm.pymap_keys(self)
    def values(self): return _ialgorithm.pymap_values(self)
    def items(self): return _ialgorithm.pymap_items(self)
    def __contains__(self, *args): return _ialgorithm.pymap___contains__(self, *args)
    def key_iterator(self): return _ialgorithm.pymap_key_iterator(self)
    def value_iterator(self): return _ialgorithm.pymap_value_iterator(self)
    def __iter__(self): return self.key_iterator()
    def iterkeys(self): return self.key_iterator()
    def itervalues(self): return self.value_iterator()
    def iteritems(self): return self.iterator()
    def __setitem__(self, *args): return _ialgorithm.pymap___setitem__(self, *args)
    def __init__(self, *args): 
        this = _ialgorithm.new_pymap(*args)
        try: self.this.append(this)
        except: self.this = this
    def empty(self): return _ialgorithm.pymap_empty(self)
    def size(self): return _ialgorithm.pymap_size(self)
    def clear(self): return _ialgorithm.pymap_clear(self)
    def swap(self, *args): return _ialgorithm.pymap_swap(self, *args)
    def get_allocator(self): return _ialgorithm.pymap_get_allocator(self)
    def begin(self): return _ialgorithm.pymap_begin(self)
    def end(self): return _ialgorithm.pymap_end(self)
    def rbegin(self): return _ialgorithm.pymap_rbegin(self)
    def rend(self): return _ialgorithm.pymap_rend(self)
    def count(self, *args): return _ialgorithm.pymap_count(self, *args)
    def erase(self, *args): return _ialgorithm.pymap_erase(self, *args)
    def find(self, *args): return _ialgorithm.pymap_find(self, *args)
    def lower_bound(self, *args): return _ialgorithm.pymap_lower_bound(self, *args)
    def upper_bound(self, *args): return _ialgorithm.pymap_upper_bound(self, *args)
    __swig_destroy__ = _ialgorithm.delete_pymap
    __del__ = lambda self : None;
pymap_swigregister = _ialgorithm.pymap_swigregister
pymap_swigregister(pymap)

class IAlgorithm(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, IAlgorithm, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, IAlgorithm, name)
    def __init__(self, *args, **kwargs): raise AttributeError("No constructor defined - class is abstract")
    __repr__ = _swig_repr
    def SetProperty(self, *args):
        """
        SetProperty(self, GML::Utils::AttributeList config) -> bool
        SetProperty(self, char config) -> bool
        """
        return _ialgorithm.IAlgorithm_SetProperty(self, *args)

    def GetProperty(self, *args):
        """GetProperty(self, GML::Utils::AttributeList config) -> bool"""
        return _ialgorithm.IAlgorithm_GetProperty(self, *args)

    def Init(self):
        """Init(self) -> bool"""
        return _ialgorithm.IAlgorithm_Init(self)

    def OnExecute(self, *args):
        """OnExecute(self, char command)"""
        return _ialgorithm.IAlgorithm_OnExecute(self, *args)

    def Execute(self, *args):
        """Execute(self, char command) -> bool"""
        return _ialgorithm.IAlgorithm_Execute(self, *args)

    def Wait(self, *args):
        """
        Wait(self) -> bool
        Wait(self, unsigned int nrMiliseconds) -> bool
        """
        return _ialgorithm.IAlgorithm_Wait(self, *args)

    def Set(self, *args):
        """Set(self, pymap v) -> bool"""
        return _ialgorithm.IAlgorithm_Set(self, *args)

    __swig_destroy__ = _ialgorithm.delete_IAlgorithm
    __del__ = lambda self : None;
IAlgorithm_swigregister = _ialgorithm.IAlgorithm_swigregister
IAlgorithm_swigregister(IAlgorithm)


