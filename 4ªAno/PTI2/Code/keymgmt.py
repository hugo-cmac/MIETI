import sys
import os
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives.asymmetric import rsa, padding
from cryptography.hazmat.primitives import serialization, hashes

def w(n, d):
    f = open(n, 'wb')
    f.write(d)
    f.close()

def g(n, p):
    s = rsa.generate_private_key(               public_exponent = 65537,
                                                key_size = 2048,
                                                backend = default_backend())

    ks = s.private_bytes(                       encoding = serialization.Encoding.PEM,
                                                format = serialization.PrivateFormat.PKCS8,
                                                encryption_algorithm = serialization.BestAvailableEncryption(p))
    w('priv/' + n + '.pem', ks)
    ks = s.public_key().public_bytes(           encoding=serialization.Encoding.PEM,
                                                format=serialization.PublicFormat.SubjectPublicKeyInfo)
    w('pub/' + n + '.pem', ks)

def s(n, p):
    f = open('../priv/' + n + '.pem', 'rb')
    s = serialization.load_pem_private_key(     f.read(),
                                                password=p,
                                                backend=default_backend())
    f.close()
    t = s.private_bytes(                        encoding=serialization.Encoding.PEM,
                                                format=serialization.PrivateFormat.PKCS8,
                                                encryption_algorithm=serialization.NoEncryption()).decode("utf-8")
    print(t)

def p(n, p):
    f = open('../priv/' + n + '.pem', 'rb')
    s = serialization.load_pem_private_key(     f.read(),
                                                password=p,
                                                backend=default_backend())
    f.close()
    t = s.private_bytes(                        encoding=serialization.Encoding.PEM,
                                                format=serialization.PrivateFormat.TraditionalOpenSSL,
                                                encryption_algorithm=serialization.NoEncryption()).decode("utf-8")
    print(t)

def main(argv):
    if argv[0] == "-c":
        if len(argv) == 3:
            g(argv[1], argv[2].encode())
    if argv[0] == "-s":
        if len(argv) == 3:
            s(argv[1], argv[2].encode())
    if argv[0] == "-p":
        if len(argv) == 3:
            p(argv[1], argv[2].encode())

if __name__== "__main__":
    if len(sys.argv) > 1:
        main(sys.argv[1:])

#python3 keygen.py -c [name] [password] -> Gen key pair
#python3 keygen.py -s [name] [password] -> Get private key(PKCS8 -> No encryption)
#python3 keygen.py -s [name] [password] -> Get private key(PEM -> No encryption)