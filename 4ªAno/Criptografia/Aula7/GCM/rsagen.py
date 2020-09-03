from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.backends import default_backend



def Server():
    private_key_server = rsa.generate_private_key(
        public_exponent=65537,
        key_size=2048,
        backend=default_backend()
        )
    pem_private_server = private_key_server.private_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PrivateFormat.PKCS8,
        encryption_algorithm=serialization.BestAvailableEncryption(b'passwordServer')
        )
    public_key_server = private_key_server.public_key()

    pem_public_server = public_key_server.public_bytes( 
        encoding=serialization.Encoding.PEM, 
        format=serialization.PublicFormat.SubjectPublicKeyInfo
        )
    server_file = open("key_server.pem", "wb")
    server_file.write(pem_private_server + pem_public_server)
    print(len(pem_private_server))

def Client():
    private_key_client = rsa.generate_private_key(
        public_exponent=65537,
        key_size=2048,
        backend=default_backend()
        )
    pem_private_client = private_key_client.private_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PrivateFormat.PKCS8,
        encryption_algorithm=serialization.BestAvailableEncryption(b'passwordClient')
        )
    public_key_client = private_key_client.public_key()

    pem_public_client = public_key_client.public_bytes( 
        encoding=serialization.Encoding.PEM, 
        format=serialization.PublicFormat.SubjectPublicKeyInfo
        )
    client_file = open("key_client.pem", "wb")
    client_file.write(pem_private_client + pem_public_client)
    print(len(pem_private_client))

Server()
Client()