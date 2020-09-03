import os
import getpass
import base64

from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives import hashes, hmac
from cryptography.hazmat.backends import default_backend

def Ler_de_Ficheiro():
    file = open("msg_encriptada.txt","rb")
    msg = file.read()
    file.close
    return msg

def Chacha20(keyC, nonce, tokenC): #chave de 32bits
    algorithm = algorithms.ChaCha20(keyC, nonce)
    chacha20 = Cipher(
            algorithm,
            mode=None,
            backend=backend
    )
    decryptor = chacha20.decryptor()#objeto
    return decryptor.update(tokenC)

def HMAC(keyM, msg, tag):
    h = hmac.HMAC(keyM, hashes.SHA256(), backend=default_backend())#objeto para cifrar
    h.update(msg)#gravar
    try:
        if h.verify(tag):
            print('Mensagem intacta')
    except:
        print('Erro: Alteracoes encontrada')
        exit

def EeM(token, nonce, keyC, keyM):
    tag = token[0:32]
    tokenC = token[32:]
    msg = Chacha20( keyC, nonce, tokenC)
    HMAC( keyM, msg, tag)#confirmacao
    return msg

def EtM(token, nonce, keyC, keyM):
    tag = token[0:32]
    tokenC = token[32:]
    HMAC( keyM, tokenC, tag)#confirmacao
    return Chacha20( keyC, nonce, tokenC)

def MtE(token, nonce, keyC, keyM):
    msgCtag = Chacha20( keyC, nonce, token)
    tag = msgCtag[0:32]
    msg = msgCtag[32:]
    HMAC( keyM, msg, tag)#confirmacao
    return msg

def Gravar_Ficheiro(msg):
    file = open("msg_desencriptada.txt","wb") 
    file.write(msg)
    file.close

backend = default_backend()
token = Ler_de_Ficheiro()
salt=token[0:32]
nonce=token[32:48]
mensagem_encriptada=token[48:]

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

print ('1-Encrypt&Mac\n2-Encrypt then Mac\n3-Mac then Encrypt\n')
op=input('Opcao: ')

if op==1:
    Gravar_Ficheiro(EeM(mensagem_encriptada, nonce, keyC, keyM))
elif op==2:
    Gravar_Ficheiro(EtM(mensagem_encriptada, nonce, keyC, keyM))
elif op==3:
    Gravar_Ficheiro(MtE(mensagem_encriptada, nonce, keyC, keyM))

