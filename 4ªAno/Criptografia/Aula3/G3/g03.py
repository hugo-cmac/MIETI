import os
import getpass
import base64

from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives import hashes, hmac
from cryptography.hazmat.backends import default_backend

def Ler_de_Ficheiro():
    file = open("msg_a_encriptar.txt","rb")
    msg = file.read()
    file.close
    return msg

def Chacha20(keyC, nonce, msg): #chave de 32bits
    algorithm = algorithms.ChaCha20(keyC, nonce)
    chacha20 = Cipher(
            algorithm,
            mode=None,
            backend=backend
    )
    encryptor = chacha20.encryptor()#objeto
    return encryptor.update(msg)

def HMAC(keyM, msg):
    h = hmac.HMAC(keyM, hashes.SHA256(), backend=default_backend())#objeto para cifrar
    h.update(msg)#gravar
    return h.finalize()#finalizacao, nao permite metodos do objeto #aqui gera o tag
    
def EeM(msg, nonce, keyC, keyM):
    tokenC = Chacha20( keyC, nonce, msg)
    tag = HMAC( keyM, msg)
    return (tag + tokenC)

def EtM(msg, nonce, keyC, keyM):
    tokenC = Chacha20( keyC, nonce, msg)
    tag = HMAC( keyM, tokenC)
    return (tag + tokenC)

def MtE(msg, nonce, keyC, keyM):
    tag = HMAC( keyM, msg)
    msg = tag + msg 
    return Chacha20( keyC, nonce, msg)

def Gravar_Ficheiro(token, salt, nonce):
    file = open("msg_encriptada.txt","wb")
    criptograma = salt + nonce + token #salt 32b + nonce 16b + tag 32b + msg 
    file.write(criptograma)
    file.close

backend = default_backend()
salt=os.urandom(32)
nonce = os.urandom(16)

kdf = PBKDF2HMAC(
    algorithm=hashes.SHA256(),
    length=64,
    salt=salt,
    iterations=100000,
    backend=backend
)

palavrapasse = getpass.getpass(prompt='Password: ', stream=None).encode()
key = kdf.derive(palavrapasse)
del palavrapasse
#separacao
keyC=key[0:32]
keyM=key[32:64]

msg = Ler_de_Ficheiro()
print '1-Encrypt&Mac\n2-Encrypt then Mac\n3-Mac then Encrypt\n'
op=input('Opcao: ')

if op==1:
    token=EeM(msg, nonce, keyC, keyM)
    Gravar_Ficheiro(token, salt, nonce)
if op==2:
    token=EtM(msg, nonce, keyC, keyM)
    Gravar_Ficheiro(token, salt, nonce)
if op==3:
    token=MtE(msg, nonce, keyC, keyM)
    Gravar_Ficheiro(token, salt, nonce)