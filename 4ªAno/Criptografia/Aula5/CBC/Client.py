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
            length=64,
            salt=self.salt,
            iterations=100000,
            backend=self.backend
        )
        return kdf.derive(b"palavrapasse")

    def Encript(self, key, plaintext):
        iv = os.urandom(16)
        cipher = Cipher(algorithms.AES(key[:32]), modes.CBC(iv), backend=self.backend)
        encryptor = cipher.encryptor()
        h = hmac.HMAC(key[32:], hashes.SHA256(), backend=self.backend)
        ct = encryptor.update(plaintext) + encryptor.finalize()
        h.update(ct)
        return (h.finalize() + iv + ct)

    def Decrypt(self, key, tag, iv, ciphertext):
        h = hmac.HMAC(key[32:], hashes.SHA256(), backend=self.backend)
        h.update(ciphertext)
        try:
            h.verify(tag)
            cipher = Cipher(algorithms.AES(key[:32]), modes.CBC(iv), backend=self.backend)
            decryptor = cipher.decryptor()
            return  (decryptor.update(ciphertext) + decryptor.finalize())
        except InvalidSignature as InvalidSignature:
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
        self.c = Cifra(salt)
        self.k = self.c.KeyGen()
    def process(self, msg=b""):
        """ Processa uma mensagem (`bytestring`) enviada pelo SERVIDOR.
            Retorna a mensagem a transmitir como resposta (`None` para
            finalizar ligacao) """
        self.msg_cnt +=1
        #
        if self.msg_cnt == 1:
            return salt
        elif self.msg_cnt == 2:
            msg=0
        else:
            msg = self.c.Decrypt(self.k, msg[:32], msg[32:48], msg[48:])
            msg = self.c.Unpadding(msg)
            print('Received (%d): %r' % (self.msg_cnt , msg.decode()))
        #
        print('Input message to send (empty to finish)')
        new_msg = input().encode()
        #
        new_msg = self.c.Padding(new_msg)
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