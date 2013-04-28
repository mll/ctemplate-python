#!/usr/bin/python
# *
# * C Template Library 1.0
# *
# * Copyright 2009 Stephen C. Losen.  Distributed under the terms
# * of the GNU General Public License (GPL)
# * 
# * Python port written by Marek Lipert (marek.lipert@gmail.com)
# *   
# *




from distutils.core import setup, Extension
import numpy.distutils.misc_util

setup(name="_ctemplate",version="1.0",
    ext_modules=[Extension("_ctemplate", ["_ctemplate.c", "ctemplate.c"])]
)