import asyncio
import os
import getpass
import base64
import random

from cryptography.hazmat.primitives import hashes, hmac, padding, serialization
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives.asymmetric import dh
from cryptography.hazmat.primitives.kdf.hkdf import HKDF
from cryptography.hazmat.backends import default_backend

conn_cnt = 0
conn_port = 8888
max_msg_size = 9999

P = 99494096650139337106186933977618513974146274831566768179581759037259788798151499814653951492724365471316253651463342255785311748602922458795201382445323499931625451272600173180136123245441204133515800495917242011863558721723303661523372572477211620144038809673692512025566673746993593384600667047373692203583
G = 44157404837960328768872680677686802650999163226766694797650810379076416463147265401084491113667624054557335394761604876882446924929840681990106974314935015501571333024773172440352475358750668213444607353872754650805031912866692119819377041901642732455911509867728218394542745330014071040326856846990119719675


class Diffie:
    def __init__(self):
        self.backend = default_backend()
        self.pn = dh.DHParameterNumbers(P,G)
        self.parameters = self.pn.parameters(self.backend)
        self.private_key = self.parameters.generate_private_key()

    def Public_key(self):
        pk = self.private_key.public_key()
        return pk.public_bytes( encoding=serialization.Encoding.PEM, format=serialization.PublicFormat.SubjectPublicKeyInfo)

    def KeyGen(self, pem):
        public_key = serialization.load_pem_public_key( pem, backend=default_backend())
        shared_key = self.private_key.exchange(public_key)
        derived_key = HKDF(
            algorithm=hashes.SHA256(),
            length=64,
            salt=None,
            info=b'handshake data',
            backend=default_backend()
        ).derive(shared_key)
        return derived_key

class Cifra:
    def __init__(self):
        self.backend = default_backend()

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

class ServerWorker(object):
    """ Classe que implementa a funcionalidade do SERVIDOR. """
    def __init__(self, cnt, addr=None):
        """ Construtor da classe. """
        self.id = cnt
        self.addr = addr
        self.msg_cnt = 0
        self.d = Diffie()
        self.k = None
        self.c = Cifra()

    def process(self, msg):
        """ Processa uma mensagem (`bytestring`) enviada pelo CLIENTE.
            Retorna a mensagem a transmitir como resposta (`None` para
            finalizar ligacao) """
        self.msg_cnt += 1
        #
        if self.msg_cnt == 1:
            new_msg = self.d.Public_key()
            self.k = self.d.KeyGen(msg)
            return new_msg if len(new_msg)>0 else None
        
        msg = self.c.Decrypt( self.k, msg[:32], msg[32:48], msg[48:])
        msg = self.c.Unpadding(msg)
        print('Received (%d): %r' % (self.msg_cnt , msg.decode()))
        #        
        txt = msg.decode()
        print('%d : %r' % (self.id,txt))
        new_msg = txt.upper().encode()
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

#Ctr  gcm