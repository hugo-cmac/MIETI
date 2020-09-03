from cryptography.fernet import Fernet

def Setup():
    file = open("chave.txt","wb")
    chave = Fernet.generate_key()
    file.write(chave)
    file.close

def Leitura_de_chave():
    file = open("chave.txt","rb")
    chave = file.read()
    file.close
    usar = Fernet(chave)
    return usar

def Ler_de_Ficheiro():
    file = open("msg_a_encriptar.txt","r")
    msg = file.read()
    file.close
    print 'Mensagem: ',msg,'\n'
    return msg

def Encriptar(msg):
    usar = Leitura_de_chave()
    token = usar.encrypt(msg)
    print 'Mensagem encriptada: ',token,'\n'
    return token

def Gravar_no_ficheiro(token):
    usar = Leitura_de_chave()
    desencriptada = usar.decrypt(token)
    file = open("msg_desencriptada.txt","wb")
    file.write(desencriptada)
    file.close
    print 'Mensagem Desencriptada: ',desencriptada,'\n'

def Ler_Mensagem():
    msg=raw_input('Escreva a mensagem a encriptar: ')
    type(msg)
    print 'Mensagem: ',msg,'\n'
    return msg

Setup()

print '1-Escrever mensagem \n2-Ler do ficheiro '
op=input('Opcao: ')

if op==1:
    msg = Ler_Mensagem()
    token = Encriptar(msg)
    Gravar_no_ficheiro(token)
if op==2:
    msg = Ler_de_Ficheiro()
    token = Encriptar(msg)
    Gravar_no_ficheiro(token)   
