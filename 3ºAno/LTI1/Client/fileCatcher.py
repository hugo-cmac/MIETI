from watchdog.observers import Observer
import time
import threading
from watchdog.events import FileSystemEventHandler
import os 
import json
import shutil
import collections
from datetime import datetime
from time import gmtime, strftime


last=""

class Py2c:
    def __init__(self, files, ourDir):
        self.file_list = files
        self.ourDir = ourDir

    def comm(self, filepath):
        self.f = open("fifo", "w")
        self.f.write(filepath)
        self.f.close()

    def typeofchange(self, typ, f):
        global last
        filepath = typ + f[len(self.ourDir):] + "\n"
        if filepath[:-1] != ("-a/Cloud/" + last):
            print(filepath)
            self.comm(filepath)

    def any_change(self, files):
        if not (len(files.keys()-self.file_list.keys()) is 0):#novo ficheiro
            changes = files.keys()-self.file_list.keys()
            for key in changes:
                self.file_list[key] = files[key]
                self.typeofchange("-a",key)
        elif not (len(self.file_list.keys()-files.keys()) is 0):#ficheiro eliminado
            changes = self.file_list.keys()-files.keys()
            for key in changes:
                self.file_list.pop(key)
                self.typeofchange("-d",key)
        else:
            for key,value in files.items():
                if key in self.file_list:
                    if value != self.file_list[key]:
                        self.file_list[key] = value
                        self.typeofchange("-u",key)

class CloudHandler(FileSystemEventHandler):
    def __init__(self, folder,ourDir):
        self.folder = folder
        self.file_list = {}
        for f in os.listdir(folder):
            self.file_list[folder + "/" + f] = os.stat(folder + "/" + f).st_size
        self.c = Py2c(self.file_list.copy(), ourDir)

    def on_modified(self, event):
        global last
        if event.src_path == self.folder:
            self.file_list.clear()
            l = os.listdir(self.folder)
            for f in l:
                if f[0] == '.':
                    last = f[1:]
                    os.rename("Cloud/" + f, "Cloud/" + last)
                    self.file_list.clear()
                    return None
                else:
                    self.file_list[self.folder + "/" + f] = os.stat(self.folder + "/" + f).st_size
            self.c.any_change(self.file_list)

    

folder = "/home/sr-d1r3171nh0/Documents/DoisBackup/3ºAno/LTI1/Final/Client/Cloud"
ourDir = "/home/sr-d1r3171nh0/Documents/DoisBackup/3ºAno/LTI1/Final/Client"
handler = CloudHandler(folder,ourDir)
obs = Observer()
obs.schedule(handler, folder, recursive=True)
obs.start()

try:
    while True:
        time.sleep(10)
except KeyboardInterrupt:
    obs.stop()
    
obs.join()