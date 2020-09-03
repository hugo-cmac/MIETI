import asyncio
import socket
import os
import getpass
import base64
import random
import datetime

from cryptography.hazmat.primitives import hashes, hmac, padding, serialization
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives.asymmetric import dh, rsa, padding
from cryptography.hazmat.primitives.kdf.hkdf import HKDF
from cryptography.hazmat.backends import default_backend
from OpenSSL import crypto
from cryptography import x509

class MyX509Cert:
    def __init__(self):
        self.ca_key = crypto.PKey()
        self.ca_key.generate_key(crypto.TYPE_RSA, 2048)

        self.ca_cert = crypto.X509()
        self.ca_cert.set_version(2)
        self.ca_cert.set_serial_number(1)

        self.ca_cert.get_subject().CN = "MyEC"
        self.ca_cert.get_subject().ST = "Portugal"
        self.ca_cert.get_subject().L = "Guimarães"
        self.ca_cert.get_subject().OU = "UMinho"

        self.ca_cert.set_issuer( self.ca_cert.get_subject())
        self.ca_cert.set_pubkey( self.ca_key)
        
        self.ca_cert.add_extensions([       crypto.X509Extension(b"subjectKeyIdentifier",   False,   b"hash",               subject=self.ca_cert),])

        self.ca_cert.add_extensions([       crypto.X509Extension(b"authorityKeyIdentifier", False,   b"keyid:always,issuer", issuer=self.ca_cert),])

        self.ca_cert.add_extensions([       crypto.X509Extension(b"basicConstraints",       True,    b"CA:TRUE"),
                                            crypto.X509Extension(b"keyUsage",               True,    b"digitalSignature, keyCertSign, cRLSign"),
                                    ])

        self.ca_cert.gmtime_adj_notBefore(0)
        self.ca_cert.gmtime_adj_notAfter(24*60*60)

        self.ca_cert.sign( self.ca_key, 'sha256')

    def __create_signed_cert(self, identity):
        k = crypto.PKey()
        k.generate_key(crypto.TYPE_RSA, 2048)

        cert = crypto.X509()
        cert.set_version(2)
        cert.set_serial_number(random.randint(50000000, 100000000))

        cert.get_subject().CN = identity
        cert.set_issuer(self.ca_cert.get_subject())
        cert.set_pubkey(k)
        
        cert.add_extensions([       crypto.X509Extension(b"basicConstraints",       False,  b"CA:FALSE"),])

        cert.add_extensions([       crypto.X509Extension(b"authorityKeyIdentifier", False,  b"keyid",           issuer=self.ca_cert),
                                    crypto.X509Extension(b"extendedKeyUsage",       False,  b"serverAuth"),
                                    crypto.X509Extension(b"keyUsage",               True,   b"digitalSignature, keyEncipherment"),
                                   ])

        cert.add_extensions([       crypto.X509Extension(b"subjectKeyIdentifier",   False,  b"hash",            subject=cert),])

        cert.gmtime_adj_notBefore(0)
        cert.gmtime_adj_notAfter(24*60*60)
        
        cert.sign(self.ca_key, 'sha256')

        keystore = crypto.PKCS12()
        keystore.set_privatekey(k)
        keystore.set_certificate(cert)
        with open(identity + ".p12", "wb") as f:
            f.write(keystore.export(passphrase="1234", iter=2048, maciter=1))
    def __saveEC(self):
        with open("EC_self_signed.cer", "wb") as f:
            f.write(crypto.dump_certificate(crypto.FILETYPE_ASN1, self.ca_cert))

    def Start(self):
        self.__create_signed_cert("Server")
        self.__create_signed_cert("Client")
        self.__saveEC()

x = MyX509Cert()
x.Start()

