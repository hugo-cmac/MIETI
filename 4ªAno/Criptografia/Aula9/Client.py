import asyncio
import socket
import os
import getpass
import base64
import random

from cryptography.hazmat.primitives import hashes, hmac, padding, serialization
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives.asymmetric import dh, rsa, padding
from cryptography.hazmat.primitives.kdf.hkdf import HKDF
from cryptography.hazmat.backends import default_backend
from OpenSSL import crypto
from cryptography import x509

conn_port = 8888
max_msg_size = 9999


P = 99494096650139337106186933977618513974146274831566768179581759037259788798151499814653951492724365471316253651463342255785311748602922458795201382445323499931625451272600173180136123245441204133515800495917242011863558721723303661523372572477211620144038809673692512025566673746993593384600667047373692203583
G = 44157404837960328768872680677686802650999163226766694797650810379076416463147265401084491113667624054557335394761604876882446924929840681990106974314935015501571333024773172440352475358750668213444607353872754650805031912866692119819377041901642732455911509867728218394542745330014071040326856846990119719675

class SSL:
    def __init__(self):
        self.p12 = crypto.load_pkcs12(open("Client.p12", 'rb').read(), "1234")
        self.cert_pem = crypto.dump_certificate(crypto.FILETYPE_PEM, self.p12.get_certificate())
        self.private_key = serialization.load_pem_private_key(
                                crypto.dump_privatekey(
                                    crypto.FILETYPE_PEM, self.p12.get_privatekey()), 
                                    None, 
                                    default_backend())
        self.trusted_cert = crypto.load_certificate(
                                    crypto.FILETYPE_PEM, 
                                    crypto.dump_certificate(
                                            crypto.FILETYPE_PEM, 
                                            crypto.load_certificate(
                                                    crypto.FILETYPE_ASN1, 
                                                    open("EC_self_signed.cer", 'rb').read())))
    def getCert(self):
        return self.cert_pem

    def SignKeys(self, client_public_key, server_public_key):
        signature = self.private_key.sign(
            (client_public_key + server_public_key),
            padding.PSS(
                mgf=padding.MGF1(hashes.SHA256()),
                salt_length=padding.PSS.MAX_LENGTH),    hashes.SHA256())
        return signature

    def verifySignature(self, signatureClient, client_public_key, server_public_key, unknown_cert):
        try:
            uc = crypto.load_certificate(crypto.FILETYPE_PEM, unknown_cert)
            store = crypto.X509Store()
            store.add_cert(self.trusted_cert)
            store_ctx = crypto.X509StoreContext(store, uc)
            store_ctx.verify_certificate()
            
            public_key_cert = x509.load_pem_x509_certificate(
                                    unknown_cert, 
                                    default_backend() ).public_key()
            public_key_cert.verify(
                    signatureClient,
                    (client_public_key + server_public_key),
                    padding.PSS(
                            mgf=padding.MGF1(hashes.SHA256()),
                            salt_length=padding.PSS.MAX_LENGTH), hashes.SHA256())

        except Exception as e:
            print(e)  
            exit

class Diffie:
    def __init__(self):
        self.private_key = dh.DHParameterNumbers( P, G).parameters( default_backend()).generate_private_key()

    def Public_key(self):
        pk = self.private_key.public_key()
        return pk.public_bytes( 
            encoding=serialization.Encoding.PEM, 
            format=serialization.PublicFormat.SubjectPublicKeyInfo
            )

    def KeyGen(self, pem):
        public_key = serialization.load_pem_public_key( pem, backend=default_backend())
        shared_key = self.private_key.exchange(public_key)
        derived_key = HKDF(
            algorithm=hashes.SHA256(),
            length=32,
            salt=None,
            info=b'handshake data',
            backend=default_backend()
        ).derive(shared_key)
        return derived_key

class Cifra:
    def __init__(self):
        self.backend = default_backend()
        
    def Encript(self, key, plaintext):  
        iv = os.urandom(32)  
        encryptor = Cipher( algorithms.AES(key), modes.GCM(iv), backend=self.backend).encryptor()
        ciphertext = encryptor.update(plaintext) + encryptor.finalize()
        return (iv + encryptor.tag + ciphertext)

    def Decrypt(self, key, iv, tag, ciphertext):
        decryptor = Cipher( algorithms.AES(key), modes.GCM(iv, tag), backend=self.backend).decryptor()
        return (decryptor.update(ciphertext) + decryptor.finalize())

class Client:
    """ Classe que implementa a funcionalidade de um CLIENTE. """
    def __init__(self, sckt=None):
        """ Construtor da classe. """
        self.sckt = sckt
        self.msg_cnt = 0
        self.d = Diffie()
        self.c = Cifra()
        self.s = SSL()
        self.public_key_server = None
        self.k = None

    def process(self, msg=b""):
        """ Processa uma mensagem (`bytestring`) enviada pelo SERVIDOR.
            Retorna a mensagem a transmitir como resposta (`None` para
            finalizar ligacao) """
        self.msg_cnt +=1
        #
        if self.msg_cnt == 1:
            return self.d.Public_key()
        elif self.msg_cnt == 2:
            self.public_key_server = msg[1456:]
            self.s.verifySignature(msg[:256], self.d.Public_key(), self.public_key_server, msg[256:1456])
            s = self.s.SignKeys( self.d.Public_key(), self.public_key_server,)
            return (s + self.s.getCert())
        elif self.msg_cnt == 3:
            self.k = self.d.KeyGen(self.public_key_server)
            print('Received (%d): %r' % (self.msg_cnt , msg.decode()))
        else:
            msg = self.c.Decrypt(self.k, msg[:32], msg[32:48], msg[48:])
            print('Received (%d): %r' % (self.msg_cnt , msg.decode()))
        #
        print('Input message to send (empty to finish)')
        new_msg = input().encode()
        #
        if len(new_msg)>0:
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