#!/usr/bin/python
import os
import glob
import sys
import re

def load_path_list(global_config):
    path_list = []
    path_dict = {}
    fileH = open(global_config)
    for line in fileH.readlines():
        comment_start = line.find("#")
        if comment_start > -1:
            line = line[:comment_start]

        if line.find("=") > -1:
            (var , path) = line.split("=")[0:2]
            path_list.append(path.strip())
            path_dict[var.strip()] = path.strip()
    return (path_list , path_dict)



(path_list , path_dict) = load_path_list("global_config")

for path in path_list:
    cwd = os.getcwd()
    if os.path.exists("%s/src/makefile" % path):
        print path
        os.chdir("%s/src" % path)
        os.system("make -s clean")
        nCPU = 4
        if path.find("sample") != -1:
            nCPU = 1
        elif path.find("analysis") != -1:
            nCPU = 1
        os.system("make -s -j %d" % (nCPU)) 
        os.chdir(cwd)


