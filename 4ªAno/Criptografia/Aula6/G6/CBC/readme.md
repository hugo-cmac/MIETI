# Guião 6: Protocolo Diffie-Hellman

Relembre o protocolo de acordo de chaves Diffie_Hellman:

    1.Alice → Bob   : g^x
    2.Bob   → Alice : g^y
    3.Alice,  Bob   : K = g^(x*y)

Onde g é um gerador de um grupo cíclico de ordem prima p, x e y são elementos aleatórios do grupo, e K é o segredo estabelecido pelo protocolo. Todas as operaçes são realizadas módulo p.

Os valores P e G são fornecido pelo docente.

#### Objectivos 

Para este guião, foi nos proposto que utilizassemos o Protocolo Diffie Hellman para efetuar o acordo de chaves. Este através da partilha das chaves publica consegue gerar um segredo em comum (g^(x*y) = X*g^y = y*g^x).

#### Opções tomadas e técnicas utilizadas

As técnicas utilizadas foram as mesmas do guião anterior, com integração do protocolo de DIffie Hellman:

-Usamos o modos CBC do AES e HMAC, para encriptar as mensagens:
    -Este necessita a utilização de PADDING para o seu funcionamento. O PADDING  utilizado foi o PKCS7.
-Par de chaves de DH.

As entidades envolvidas geram os seu par de chaves com os parametros P e G, e cada entidade partilha a sua de publica.
Depois com a propria chave privada e a chave publica da outra entidades é possivel calcular uma chave comum.


#### Instruções de Uso

Para a sua utilização é so correr em terminais diferenter os seguintes comandos:
$python3 Server.py
$python3 Client.py


#### Ultima alteração em 14/12/2019