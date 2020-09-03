import requests
import time
import threading


def comm(c):
    f = open("fromWeb", "w")
    f.write(c)
    f.close()

def checkNew():#new
    r = requests.get('http://127.0.0.1:11111/api/newfile')
    filestring = str(r.content.decode("utf-8"))
    if filestring != "false":
        print(filestring)
        c = "-a/Cloud/" + filestring + "\n"
        comm(c)

def checkDelete():#delete
    r = requests.get('http://127.0.0.1:11111/api/deletedfile')
    filestring = str(r.content.decode("utf-8"))
    if filestring != "false":
        print(filestring)
        c = "-d/Cloud/" + filestring + "\n"
        comm(c)

def fromWeb():
    b = True
    while b:
        checkDelete()
        checkNew()
        time.sleep(5)


def deletefile(file):
    r = requests.delete('http://127.0.0.1:11111/api/file/' + file)

def toWeb():
    b = True
    f = open("toWeb", "r")
    while b:
        c = f.read()
        deletefile(c[8:])



if __name__ == "__main__":
    for i in range(2):
        if i == 0:
            x = threading.Thread(target=fromWeb, args=( ))
        else:
            x = threading.Thread(target=toWeb, args=( ))
        x.start()
    
    x.join()
