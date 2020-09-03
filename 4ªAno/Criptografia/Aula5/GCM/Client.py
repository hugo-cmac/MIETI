import asyncio
import socket
import os
import getpass
import base64
import random

from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
from cryptography.hazmat.primitives import hashes, hmac
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import padding

conn_port = 8888
max_msg_size = 9999
salt = os.urandom(32)

class Cifra:
    def __init__(self,salt):
        self.backend = default_backend()
        self.salt = salt
        
    def KeyGen(self):
        kdf = PBKDF2HMAC(
            algorithm=hashes.SHA256(),
            length=32,
            salt=self.salt,
            iterations=100000,
            backend=self.backend
        )
        return kdf.derive(b"palavrapasse")

    def Encript(self, key, plaintext):  
        iv = os.urandom(32)  
        encryptor = Cipher( algorithms.AES(key), modes.GCM(iv), backend=self.backend).encryptor()
        ciphertext = encryptor.update(plaintext) + encryptor.finalize()
        return (iv + encryptor.tag + ciphertext)

    def Decrypt(self, key, iv, tag,ciphertext):
        decryptor = Cipher( algorithms.AES(key), modes.GCM(iv, tag), backend=self.backend).decryptor()
        return (decryptor.update(ciphertext) + decryptor.finalize())


class Client:
    """ Classe que implementa a funcionalidade de um CLIENTE. """
    def __init__(self, sckt=None):
        """ Construtor da classe. """
        self.sckt = sckt
        self.msg_cnt = 0
        self.c = Cifra(salt)
        self.k = self.c.KeyGen()

    def process(self, msg=b""):
        """ Processa uma mensagem (`bytestring`) enviada pelo SERVIDOR.
            Retorna a mensagem a transmitir como resposta (`None` para
            finalizar ligacao) """
        self.msg_cnt +=1
        #
        if self.msg_cnt == 1:
            new_msg = salt
            return new_msg if len(new_msg)>0 else None
        elif self.msg_cnt == 2:
            msg=0
        else:
            msg = self.c.Decrypt(self.k, msg[:32], msg[32:48], msg[48:])
            print('Received (%d): %r' % (self.msg_cnt , msg.decode()))
        #
        print('Input message to send (empty to finish)')
        new_msg = input().encode()
        #
        new_msg = self.c.Encript(self.k, new_msg)
        #
        return new_msg if len(new_msg)>0 else None



#
#
# Funcionalidade Cliente/Servidor
#
# obs: nao devera ser necessario alterar o que se segue
#


@asyncio.coroutine
def tcp_echo_client(loop=None):
    if loop is None:
        loop = asyncio.get_event_loop()

    reader, writer = yield from asyncio.open_connection('127.0.0.1', conn_port, loop=loop)
    addr = writer.get_extra_info('peername')
    client = Client(addr)
    msg = client.process()
    while msg:
        writer.write(msg)
        msg = yield from reader.read(max_msg_size)
        if msg :
            msg = client.process(msg)
        else:
            break
    writer.write(b'\n')
    print('Socket closed!')
    writer.close()

def run_client():
    loop = asyncio.get_event_loop()
    loop.run_until_complete(tcp_echo_client())


run_client()