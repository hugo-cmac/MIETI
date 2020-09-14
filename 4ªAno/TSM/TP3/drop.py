from mutagen.easyid3 import EasyID3
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

import time
import os
import json

flag = True

def update():
    global flag
    musicList = []
    gens = os.listdir("Music/")
    for gen in gens:
        arts = os.listdir("Music/" + gen)
        for art in arts:
            albs = os.listdir("Music/" + gen + "/" + art)
            for alb in albs:
                musics = os.listdir("Music/" + gen + "/" + art + "/" + alb)
                for music in musics:
                    musicList.append("Music/" + gen + "/" + art + "/" + alb + "/" + music + "\n")
    flag = False
    f = open("music.list", "w")
    for m in musicList:
        f.write(m) 
    f.close()
    
def exists(what, where):
    if what in where:
        return "/" + where[what][0]
    else:
        return "/" + "Desconhecido"

class CloudHandler(FileSystemEventHandler):
    def on_created(self, event):
        global flag
        if not os.path.isdir(event.src_path):
            try:
                m = EasyID3(event.src_path)              
                p = "Music"

                p += exists('genre', m)
                if not os.path.isdir(p):
                    os.mkdir(p)

                p += exists('artist', m)
                if not os.path.isdir(p):
                    os.mkdir(p)

                p += exists('album', m)
                if not os.path.isdir(p):
                    os.mkdir(p)
                
                if 'title' in m:
                    p += "/" + m['title'][0] + ".wav"
                else:
                    os.remove(event.src_path)
                    return

                if str(event.src_path).endswith('.wav'):
                    os.rename(event.src_path, p)
                else:
                    r = os.system("ffmpeg -i \'" + event.src_path + "\' \'" + p + "\' -y")
                    if r == 0:
                        flag = True
                os.remove(event.src_path)
            except:
                os.remove(event.src_path)
                return
            
update()
folder = "dragNdrop"
os.mkdir(folder)


handler = CloudHandler()
obs = Observer()
obs.schedule(handler, folder, recursive=True)
obs.start()
try:
    while True:
        time.sleep(10)
        if flag:
            update()
except KeyboardInterrupt:
    obs.stop()
    os.rmdir(folder)

obs.join()
