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

conn_cnt = 0
conn_port = 8888
max_msg_size = 9999

P = 99494096650139337106186933977618513974146274831566768179581759037259788798151499814653951492724365471316253651463342255785311748602922458795201382445323499931625451272600173180136123245441204133515800495917242011863558721723303661523372572477211620144038809673692512025566673746993593384600667047373692203583
G = 44157404837960328768872680677686802650999163226766694797650810379076416463147265401084491113667624054557335394761604876882446924929840681990106974314935015501571333024773172440352475358750668213444607353872754650805031912866692119819377041901642732455911509867728218394542745330014071040326856846990119719675

class SSL:
    def __init__(self):
        self.p12 = crypto.load_pkcs12(open("Server.p12", 'rb').read(), "1234")
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
        encryptor = Cipher(algorithms.AES(key), modes.GCM(iv), backend=self.backend).encryptor()
        ciphertext = encryptor.update(plaintext) + encryptor.finalize()
        return (iv + encryptor.tag + ciphertext)

    def Decrypt(self, key, iv, tag, ciphertext):
        decryptor = Cipher( algorithms.AES(key), modes.GCM(iv, tag), backend=self.backend).decryptor()
        return (decryptor.update(ciphertext) + decryptor.finalize())

class ServerWorker(object):
    """ Classe que implementa a funcionalidade do SERVIDOR. """
    def __init__(self, cnt, addr=None):
        """ Construtor da classe. """
        self.id = cnt
        self.addr = addr
        self.msg_cnt = 0
        self.d = Diffie()
        self.c = Cifra()
        self.s = SSL()
        self.public_key_client = None
        self.k = None
 
    def process(self, msg):
        """ Processa uma mensagem (`bytestring`) enviada pelo CLIENTE.
            Retorna a mensagem a transmitir como resposta (`None` para
            finalizar ligacao) """
        self.msg_cnt += 1
        #
        if self.msg_cnt == 1:
            self.public_key_client = msg
            s = self.s.SignKeys( self.public_key_client, self.d.Public_key())
            return (s + self.s.getCert() + self.d.Public_key())
        elif self.msg_cnt == 2:
            self.s.verifySignature(msg[:256], self.public_key_client, self.d.Public_key(), msg[256:])
            self.k = self.d.KeyGen(self.public_key_client)
            return (b'Server Ready')
        else:
            msg = self.c.Decrypt(self.k, msg[:32], msg[32:48], msg[48:])
        #        
        txt = msg.decode()
        print('%d : %r' % (self.id,txt))
        new_msg = txt.upper().encode()
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