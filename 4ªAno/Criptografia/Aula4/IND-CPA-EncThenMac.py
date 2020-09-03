import os
import getpass
import base64
import random

from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives import hashes, hmac
from cryptography.hazmat.backends import default_backend


class Adversario:  
    def Choose(self):
        return [b"00000000",b"11111111"]
    
    def Guess(self, enc_oracle, c):
        
        if c[0:32] == enc_oracle(b"11111111")[0:32]:
            return 1

        return 0

            
        
class Cifra:
    def __init__(self):
        self.backend = default_backend()
        self.salt=os.urandom(32)
    
    def KeyGen(self):
        kdf = PBKDF2HMAC(
            algorithm=hashes.SHA256(),
            length=64,
            salt=self.salt,
            iterations=100000,
            backend=self.backend
        )
        return kdf.derive("palavrapasse")

    def Enc(self, key, plaintext):
        algoritmo = algorithms.ChaCha20(key[0:32], os.urandom(16))
        chacha20 = Cipher( algoritmo, mode=None, backend=self.backend)
        c = chacha20.encryptor()
        c1 = c.update(plaintext)
        h = hmac.HMAC(key[32:64], hashes.SHA256(), backend=default_backend())
        h.update(plaintext)
        c2 = h.finalize()
        return (c2 + c1)


def IND_CPA(cifra, adversario):
    key = cifra.KeyGen()
    enc_oracle = lambda plaintext: cifra.Enc(key,plaintext)
    m = adversario.Choose()
    bit = random.randint(0, 1)
    criptograma = cifra.Enc(key, m[bit])
    bitguess = adversario.Guess(enc_oracle, criptograma)
    if bitguess == bit:
        return True
    else:
        return False


i = 0
c = Cifra()
a = Adversario()
media = 0
while i != 100:
    i+=1
    if IND_CPA( c, a) == True:
        media+=1

print(media)