import os
import getpass
import base64

from cryptography.fernet import Fernet
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
from cryptography.hazmat.backends import default_backend

backend = default_backend() # Salts should be randomly generated
salt = os.urandom(16)
# derive
kdf = PBKDF2HMAC(   algorithm=hashes.SHA256(),
                    length=32,
                    salt=salt,
                    iterations=100000,
                    backend=backend
                )

def Ler_de_Ficheiro():
    file = open("msg_a_encriptar.txt","rb")
    msg = file.read()
    file.close
    return msg

def Encriptar(msg, key, salt):
    file = open("msg_encriptada.txt","wb")
    usar = Fernet(base64.b64encode(key))
    token = usar.encrypt(msg)
    criptograma = salt + token #salt 16bytes + mensagem encriptada
    file.write(criptograma)
    file.close


palavrapasse = getpass.getpass(prompt='Password: ', stream=None).encode()
key = kdf.derive(palavrapasse)
del palavrapasse
msg = Ler_de_Ficheiro()
Encriptar(msg, key, salt)