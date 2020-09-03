from OpenSSL import crypto
from cryptography import x509
from cryptography.hazmat.backends import default_backend

def verify():

    p12_client = crypto.load_pkcs12(open("Servidor.p12", 'rb').read(), "1234")
    
    client_certific = crypto.dump_certificate(crypto.FILETYPE_PEM, p12_client.get_certificate())
    public_key_ssl_client = x509.load_pem_x509_certificate(client_certific, default_backend()).public_key()
    print(len(client_certific))
    #p12_server = crypto.load_pkcs12(open("Servidor.p12", 'rb').read(), "1234")
    #p12_server.get_certificate()

    ca = crypto.load_certificate(crypto.FILETYPE_ASN1, open("CA.cer", 'rb').read())
    cert = crypto.dump_certificate(crypto.FILETYPE_PEM, ca)

    #manda para a funçao    
    try:
        trusted = crypto.load_certificate(crypto.FILETYPE_PEM, cert)
        c = crypto.load_certificate(crypto.FILETYPE_PEM, client_certific)

        store = crypto.X509Store()

        store.add_cert(trusted)

        store_ctx = crypto.X509StoreContext(store, c)

        store_ctx.verify_certificate()

        print("certificado confirmado")


    except Exception as e:
        print(e)
        print("com e")    

    #with open('./cert.pem', 'r') as cert_file:
    #    cert = cert_file.read()

    #with open('./int-cert.pem', 'r') as int_cert_file:
    #    int_cert = int_cert_file.read()

    #with open('./root-cert.pem', 'r') as root_cert_file:
    #    root_cert = root_cert_file .read()

    #trusted_certs = (int_cert, root_cert)
    #verified = verify_chain_of_trust(cert, trusted_certs)

    #if verified:
    #    print('Certificate verified')


def verify_chain_of_trust(cert_pem, trusted_cert_pems):

    certificate = crypto.load_certificate(crypto.FILETYPE_PEM, cert_pem)

    try:
        # Create and fill a X509Sore with trusted certs
        store = crypto.X509Store()
        for trusted_cert_pem in trusted_cert_pems:
            trusted_cert = crypto.load_certificate(crypto.FILETYPE_PEM, trusted_cert_pem)
            store.add_cert(trusted_cert)

        # Create a X590StoreContext with the cert and trusted certs
        # and verify the the chain of trust
        store_ctx = crypto.X509StoreContext(store, certificate)
        # Returns None if certificate can be validated
        store_ctx.verify_certificate()
    except Exception as e:
        print(e)
        return False

verify()