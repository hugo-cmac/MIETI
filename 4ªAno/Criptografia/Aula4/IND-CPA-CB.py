import os
import getpass
import base64
import random

from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives import hashes, hmac
from cryptography.hazmat.backends import default_backend
from Crypto.Cipher import AES


class Adversario:  
    def Choose(self):
        return [b"00000000000000000000000000000000",b"00000000000000001111111111111111"]
    
    def Guess(self, enc_oracle, c):
        
        if c[:16] == c[16:]:
            return 0

        return 1

            
        
class Cifra:
    def __init__(self):
        self.backend = default_backend()
        self.nonce = os.urandom(16)
    
    def KeyGen(self):
        kdf = PBKDF2HMAC(
            algorithm=hashes.SHA256(),
            length=16,
            salt=os.urandom(32),
            iterations=100000,
            backend=self.backend
        )
        return kdf.derive("palavrapasse")

    def Enc(self, key, plaintext):
        cipher = AES.new(key,AES.MODE_ECB)
        c = cipher.encrypt(plaintext)
        return (c)




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