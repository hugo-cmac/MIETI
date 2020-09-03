from mutagen.mp3 import MP3
import os

stl = {}
artl = {}
albl = {}
forl = {}

class Music():
    def __init__(self, styl, art, alb, nam, tim, siz, formt, pat):
        self.style = styl
        self.artist = art
        self.album = alb
        self.name = name
        self.time = tim
        self.size = siz
        self.format = formt
        self.path = pat
    
    def allmusicTable(self):
        r = ";" + str(stl[self.style]) + ";" + str(artl[self.artist]) + ";" + str(albl[self.album]) + ";" + self.name + ";" + str(self.time) + ";" + str(self.size) + ";" + str(forl[self.format])
        return r
    
    def musiclist(self):
        self.path = self.path[89:]
        r = ";" + self.path 
        return r
    

folder = "/home/sr-d1r3171nh0/Documents/DoisBackup/4ªAno/Gestão de Redes/tps/tp3/hostmusicmibFINAL/Music"
AllMusic = []

stylenr = 1
artistnr = 1
albumnr = 1
formatnr = 1

styleList = os.listdir(folder)
for style in styleList:
    folder2 = folder + "/" + style
    artistList = os.listdir(folder2)
    stl[style] = stylenr
    stylenr = stylenr + 1
    for artist in artistList:
        folder3 = folder2 + "/" + artist
        albumList = os.listdir(folder3)
        artl[artist] = artistnr
        artistnr = artistnr + 1
        for album in albumList:
            folder4 = folder3 + "/" + album
            musicList = os.listdir(folder4)
            albl[album] = albumnr
            albumnr = albumnr + 1
            for music in musicList:
                fullPath = folder4 + "/" + music
                name, ext = os.path.splitext(music)
                ext = ext[1:]
                if not any(ext in sublist for sublist in forl) :
                    forl[ext] = formatnr
                    formatnr = formatnr + 1
                size = os.stat(fullPath).st_size
                duration = int(MP3(fullPath).info.length)
                newMusic = Music(style, artist, album, name, duration, size, ext, fullPath)
                AllMusic.append(newMusic)

AllMusic.sort(key=lambda x: x.name, reverse=False)     
i=1

f1 = open("/etc/hostmusicmib.conf/allmusicTable.conf"   , "wt")
f2 = open("/etc/hostmusicmib.conf/styleTable.conf"      , "wt")
f3 = open("/etc/hostmusicmib.conf/artistTable.conf"     , "wt")
f4 = open("/etc/hostmusicmib.conf/albumTable.conf"      , "wt")
f5 = open("/etc/hostmusicmib.conf/formatTable.conf"     , "wt")
f6 = open("/etc/hostmusicmib.conf/musicList.conf"       , "wt")

for m in AllMusic:
    f1.write(str(i) + m.allmusicTable() + os.linesep)
    f6.write(str(i) + m.musiclist() + os.linesep)
    i = i + 1

for x, y in stl.items():
    f2.write(str(y) + ";" + x + os.linesep)

for x, y in artl.items():
    f3.write(str(y) + ";" + x + os.linesep)
   
for x, y in albl.items():
    f4.write(str(y) + ";" + x + os.linesep)

for x, y in forl.items():
    f5.write(str(y) + ";" + x + os.linesep)


f1.close()
f2.close()
f3.close()
f4.close()
f5.close()
f6.close()

