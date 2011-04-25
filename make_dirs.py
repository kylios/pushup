#!/usr/bin/python

# Sets up a kylux directory structure in the current directory
# mkdir kylux
# cd kylux
# ./make_dirs.py

import os

def mktree (pBaseDir):
    baseComp = pBaseDir + os.sep
    os.mkdir (baseComp + "kernel")
    os.mkdir (baseComp + "lib")
    os.mkdir (baseComp + "lib" + os.sep + "user")
    os.mkdir (baseComp + "lib" + os.sep + "kernel")
    os.mkdir (baseComp + "arch");
    os.mkdir (baseComp + "arch" + os.sep + "x86")
    os.mkdir (baseComp + "test")
    os.mkdir (baseComp + "util")

os.mkdir ("src")
os.mkdir ("include")

mktree ("src")
mktree ("include")


