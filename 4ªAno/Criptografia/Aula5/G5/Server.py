import asyncio
import os
import getpass
import base64
import random

from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
from cryptography.hazmat.primitives import hashes, hmac
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import padding


conn_cnt = 0
conn_port = 8888
max_msg_size = 9999

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
        return (h.finalize() + ct)

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

class ServerWorker(object):
    """ Classe que implementa a funcionalidade do SERVIDOR. """
    def __init__(self, cnt, addr=None):
        """ Construtor da classe. """
        self.id = cnt
        self.addr = addr
        self.msg_cnt = 0
    def process(self, msg):
        """ Processa uma mensagem (`bytestring`) enviada pelo CLIENTE.
            Retorna a mensagem a transmitir como resposta (`None` para
            finalizar ligacao) """
        self.msg_cnt += 1
        #
        
        c = Cifra(msg[:48]) 
        key = c.KeyGen()
        msg = c.Decrypt(key, msg[48:])
        msg = c.Unpadding(msg)
        #        
        txt = msg.decode()
        print('%d : %r' % (self.id,txt))
        new_msg = txt.upper().encode()
        #
        new_msg = c.Padding(new_msg)
        new_msg = c.Encript(key, new_msg)
        #
        return new_msg if len(new_msg)>0 else None


#
#
# Funcionalidade Cliente/Servidor
#
# obs: nao devera ser necessario alterar o que se segue
#


@asyncio.coroutine
def handle_echo(reader, writer):
    global conn_cnt
    conn_cnt +=1
    addr = writer.get_extra_info('peername')
    srvwrk = ServerWorker(conn_cnt, addr)
    data = yield from reader.read(max_msg_size)
    while True:
        if not data: continue
        if data[:1]==b'\n': break
        data = srvwrk.process(data)
        if not data: break
        writer.write(data)
        yield from writer.drain()
        data = yield from reader.read(max_msg_size)
    print("[%d]" % srvwrk.id)
    writer.close()


def run_server():
    loop = asyncio.get_event_loop()
    coro = asyncio.start_server(handle_echo, '127.0.0.1', conn_port, loop=loop)
    server = loop.run_until_complete(coro)
    # Serve requests until Ctrl+C is pressed
    print('Serving on {}'.format(server.sockets[0].getsockname()))
    print('  (type ^C to finish)\n')
    try:
        loop.run_forever()
    except KeyboardInterrupt:
        pass
    # Close the server
    server.close()
    loop.run_until_complete(server.wait_closed())
    loop.close()
    print('\nFINISHED!')

run_server()