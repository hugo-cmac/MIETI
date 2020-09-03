import os
import getpass
import base64

from cryptography.fernet import Fernet
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
from cryptography.hazmat.backends import default_backend

backend = default_backend() # Salts should be randomly generated

def Desencriptar(token, key):
    file = open("msg_desencriptada.txt","wb")
    usar = Fernet(base64.b64encode(key))
    msg = usar.decrypt(token)
    file.write(msg)
    file.close


palavrapasse = getpass.getpass(prompt='Password: ', stream=None).encode()
file = open("msg_encriptada.txt","rb")
criptograma = file.read()
file.close
salt = criptograma[0:16]   
token = criptograma[16:]


kdf = PBKDF2HMAC(
    algorithm=hashes.SHA256(),
    length=32,
    salt=salt,
    iterations=100000,
    backend=backend
)

key = kdf.derive(palavrapasse)
del palavrapasse
Desencriptar( token, key)