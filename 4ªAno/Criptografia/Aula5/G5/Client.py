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
saltPlusIV = os.urandom(32) + os.urandom(16)

class Cifra:
    def __init__(self,conf):
        self.backend = default_backend()
        self.salt = conf[:32]
        self.iv = conf[32:]
    
    def KeyGen(self):
        kdf = PBKDF2HMAC(
            algorithm=hashes.SHA256(),
            length=64,
            salt=self.salt,
            iterations=100000,
            backend=self.backend
        )
        return kdf.derive(b"palavrapasse")

    def Encript(self, key, plaintext):
        cipher = Cipher(algorithms.AES(key[:32]), modes.CBC(self.iv), backend=self.backend)
        encryptor = cipher.encryptor()
        h = hmac.HMAC(key[32:], hashes.SHA256(), backend=default_backend())
        ct = encryptor.update(plaintext) + encryptor.finalize()
        h.update(ct)
        return ( h.finalize() + ct)

    def Decrypt(self, key, ct):
        h = hmac.HMAC(key[32:], hashes.SHA256(), backend=default_backend())
        h.update(ct[32:])
        
        if ct[:32]==h.finalize():
            cipher = Cipher(algorithms.AES(key[:32]), modes.CBC(self.iv), backend=self.backend)
            decryptor = cipher.decryptor()
            return  (decryptor.update(ct[32:]) + decryptor.finalize())
        exit

    def Padding(self, plaintext):
        padder = padding.PKCS7(128).padder()
        padded_data = padder.update(plaintext)
        return (padded_data + padder.finalize())

    def Unpadding(self, plaintext):
        unpadder = padding.PKCS7(128).unpadder()
        data = unpadder.update(plaintext)
        return (data + unpadder.finalize())

class Client:
    """ Classe que implementa a funcionalidade de um CLIENTE. """
    def __init__(self, sckt=None):
        """ Construtor da classe. """
        self.sckt = sckt
        self.msg_cnt = 0
    def process(self, msg=b""):
        """ Processa uma mensagem (`bytestring`) enviada pelo SERVIDOR.
            Retorna a mensagem a transmitir como resposta (`None` para
            finalizar ligacao) """
        self.msg_cnt +=1
        #
        
        c = Cifra(saltPlusIV)
        key = c.KeyGen()
        if self.msg_cnt!=1:
            msg = c.Decrypt( key, msg)
            msg = c.Unpadding(msg)
        #
        print('Received (%d): %r' % (self.msg_cnt , msg.decode()))
        print('Input message to send (empty to finish)')
        new_msg = input().encode()
        #
        new_msg = c.Padding(new_msg)
        new_msg = c.Encript(key, new_msg)  
        new_msg = saltPlusIV + new_msg
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