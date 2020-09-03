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
        return [b"00000000",b"00001111"]
    
    def Guess(self, enc_oracle, c):
        i=0
        um=0
        zero=0
        while i!=50:
            i+=1
            print(c.encode('hex'))
            c = enc_oracle(c)
            if c[:4] == c[4:]:
                zero+=1
            else:
                um+=1
        print(zero," ",um)
        if zero<um:
            return 1
        else:
            return 0
        

            
        
class Cifra:
    def __init__(self):
        self.backend = default_backend()
    
    def KeyGen(self):
        kdf = PBKDF2HMAC(
            algorithm=hashes.SHA256(),
            length=32,
            salt=os.urandom(32),
            iterations=100000,
            backend=self.backend
        )
        return kdf.derive("palavrapasse")

    def Enc(self, key, plaintext):
        algoritmo = algorithms.ChaCha20(key, os.urandom(16))
        chacha20 = Cipher( algoritmo, mode=None, backend=self.backend)
        c = chacha20.encryptor()
        c1 = c.update(plaintext)
        return (c1)




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