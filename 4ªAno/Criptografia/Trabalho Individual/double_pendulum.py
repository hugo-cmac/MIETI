import sys
import time
import random
import numpy as np
import matplotlib.pyplot as plt
import warnings
import matplotlib.cbook
import binascii
from scipy.integrate import odeint
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.hkdf import HKDF
from cryptography.hazmat.backends import default_backend
warnings.filterwarnings("ignore",category=matplotlib.cbook.mplDeprecation)


class DoublePendulum:
    def __init__(self):
        self.y2 = None
        self.x2 = None
        self.pubkey = None
        self.g = 9.81
        self.t = None

    def _deriv(self, y, t, l1, l2, m1, m2):
        theta1, z1, theta2, z2 = y
        c, s = np.cos(theta1-theta2), np.sin(theta1-theta2)
        theta1dot = z1
        z1dot = (m2*self.g*np.sin(theta2)*c - m2*s*(l1*z1**2*c + l2*z2**2) - (m1 + m2)*self.g*np.sin(theta1)) / l1 / (m1 + m2*s**2)
        theta2dot = z2
        z2dot = ((m1 + m2)*(l1*z1**2*s - self.g*np.sin(theta2) + self.g*np.sin(theta1)*c) + m2*l2*z2**2*s*c) / l2 / (m1 + m2*s**2)
        return theta1dot, z1dot, theta2dot, z2dot

    def generate_curve(self):
        self.t = time.localtime(time.time())
        tmax, dt = 3, 0.001
        t = np.arange(0, tmax+dt, dt)
        y0 = np.array([(np.pi*(self.t.tm_hour + self.t.tm_min/60))/6, 0, (np.pi * (self.t.tm_min))/30, 0])
        y = odeint(self._deriv, y0, t, args=(1, 1, 1, 2))
        theta1, theta2 = y[:,0], y[:,2]
        x1 = 1 * np.sin(theta1)
        y1 = -1 * np.cos(theta1)
        self.x2 = x1 + 1 * np.sin(theta2)
        self.y2 = y1 - 1 * np.cos(theta2)

    def _long_to_int(self, value):
        wid = value.bit_length()
        wid += 8 - ((wid % 8) or 8)
        fmt = '%%0%dx' % (wid // 4)
        binascii.unhexlify(fmt % value)
        return binascii.unhexlify(fmt % value)

    def public_key(self):
        self.pubkey = random.randint(0,len(self.y2))
        return self.pubkey

    def peer_public_key(self):
        return random.randint(0,len(self.y2))

    def drawAll(self, pubkey):
        #pontos random
        a = self.pubkey
        b = pubkey

        pk1 = plt.scatter(self.x2[a], self.y2[a], c="green")
        pk2 = plt.scatter(self.x2[b], self.y2[b], c="blue")
       
        #desenho da curva
        plt.plot(self.x2,self.y2)
        plt.xlabel('x') 
        plt.ylabel('y') 
        plt.title('Minha curva')
        plt.grid()

        #desenho da reta 
        m = (self.y2[a] - self.y2[b])/(self.x2[a] - self.x2[b])
        baux = self.y2[a] - m*self.x2[a]
        x = np.linspace(min(self.x2),max(self.x2),100)
        reta = plt.subplot()
        reta.plot(x,m*x+baux)
     
        #reta perpendicular
        idx = np.argwhere(np.diff(np.sign((m*self.x2+baux) - self.y2))).flatten()
        n1 = 0
        point = 0
        for i in idx:
            if (self.x2[a]!=self.x2[i]) and (np.round(self.y2[a],1)!=np.round(self.y2[i],1)) and (self.x2[b]!=self.x2[i]) and (np.round(self.y2[b],1)!=np.round(self.y2[i],1)):    
                n1=i
                point = plt.plot(self.x2[n1], self.y2[n1], 'co')
                break
        m = -1/m
        baux = self.y2[n1] - m*self.x2[n1]
        retaperp = plt.subplot()
        retaperp.plot(self.x2,m*self.x2+baux)

        #interseção da reta perpendicular com curva
        idx = np.argwhere(np.diff(np.sign((m*self.x2+baux) - self.y2))).flatten()
        n2 = 0
        for i in idx:
            if (n1!=i) and (np.round(self.y2[n1],1)!=np.round(self.y2[i],1)):    
                n2=i
                if n2!=0:
                    break
                
        shared = plt.plot(self.x2[n2], self.y2[n2], 'ro')
        plt.legend(( pk1, pk2, point[0], shared[0]), ("Chave Pública 1", "Chave Pública 2", "Interseção","Chave Partilhada"), scatterpoints=1, loc='lower left', ncol=4, fontsize=8)
        plt.show()

    def drawCurve(self):
        plt.plot(self.x2,self.y2)
        plt.xlabel('x') 
        plt.ylabel('y') 
        plt.title('Minha curva')
        plt.grid()
        plt.show()

    def exchange(self,pubkey):#exchange
        nodec = 10000000000000000
        #reta entre os dois pontos
        a = self.pubkey
        b = pubkey
        m = (self.y2[a] - self.y2[b])/(self.x2[a] - self.x2[b])
        baux = self.y2[a] - m*self.x2[a]

        #terceiro ponto de interseçao desta reta
        idx = np.argwhere(np.diff(np.sign((m*self.x2+baux) - self.y2))).flatten()
        n1 = 0
        for i in idx:
            if (a!=i) and (np.round(self.y2[a],1)!=np.round(self.y2[i],1)) and (b!=i) and (np.round(self.y2[b],1)!=np.round(self.y2[i],1)):    
                n1=i
                break

        #reta perpendicular a reta anterior apartir do terceiro ponto
        m = -1/m
        baux = self.y2[n1] - m*self.x2[n1]

        #interseção da reta perpendicular com curva
        idx = np.argwhere(np.diff(np.sign((m*self.x2+baux) - self.y2))).flatten()
        n2 = 0
        for i in idx:
            if (self.x2[n1]!=self.x2[i]) and (np.round(self.y2[n1],1)!=np.round(self.y2[i],1)):    
                n2 = i
                if n2!=0:
                    break
        
        #multiplicaçaos dos dois pontos 
        n = int((abs(int(self.x2[n2]*nodec)) * abs(int(self.y2[n2]*nodec)))**(3))
        
        #inteiro para bytes
        byt = self._long_to_int(n)
        return byt


#Inicialização do objeto
d = DoublePendulum()

#Geração da curva
d.generate_curve()

#Geração da chave publica(escolha aleatória)
#é devolvida e fica guardada no objeto(nao necessario inclui la no exchange)
d.public_key()

#Geração de chave publica do Peer(apenas para esta demonstração) 
peer_pubkey = d.peer_public_key()

d.drawCurve()
#Metodo para desenhar curva, retas e interseções que entram na cifra
#Apenas para demonstração
d.drawAll(peer_pubkey)

#Metodo para desenhar a curva gerada
#Apenas para demonstração
#d.drawCurve()

#Metodo para a geração da chave partilhada
shared_key = d.exchange(peer_pubkey)

#Impressão do resultado
print(binascii.hexlify(bytes(shared_key)))



#shared_key = HKDF(        algorithm=hashes.SHA256(),
#                                        length=32,
#                                        salt=None,
#                                        info=b'handshake data',
#                                        backend=default_backend()
#                                ).derive(shared_key)
#print(binascii.hexlify(bytes(shared_key)))
