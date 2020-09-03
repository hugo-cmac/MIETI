# Guião 8: Protocolo StS simplificado

Pretende-se complementar o programa com o acordo de chaves Diffie-Hellman para incluir a funcionalidade análoga à do protocolo Station-to-Station. Recorde que nesse protocolo é adicionado uma troca de assinaturas:

    1.Alice → Bob   : gx
    2.Bob   → Alice : gy, SigB(gx, gy)
    3.Alice → Bob   : SigA(gx, gy)
    4.Alice,  Bob   : K = g(x*y)

De notar que um requisito adicional neste protocolo é a manipulação de pares de chaves assimétricas para realizar as assinaturas digitais (e.g. RSA). Para tal deve produzir um pequeno programa que gere os pares de chaves para cada um dos intervenientes e os guarde em ficheiros que serão lidos pela aplicação Cliente/Servidor.


#### Objectivos 

Para este guião, foi nos proposto que integrasse-mos assinaturas digitais, com o guião anterior, para assinar as chaves publicas de Diffie Hellman, de forma a garantir a autenticidade, porque os pares de chaves RSA neste caso especifico são teoricamente obtidas através de um canal seguro.


#### Opções tomadas e técnicas utilizadas

As técnicas utilizadas foram as mesmas do guião anterior, com integração das assinaturas digitais.

-Protocolo Diffie Hellman(geração de um segredo partilhado); 
-Usamos o modos GCM do AES, para encriptar as mensagens 
-Par de chaves RSA.

Aqui fizemos um pequeno script(rsagen.py) para nos gerar os pares de chaves públicas e privadas, e gravamos em formato PEM, protegidos por uma chave.
Depois na aplicação cliente e servidor lemos o respectivo ficheiro, com as respetivas chaves.
Para o precesso de verificação da assinatura o programa vai buscar a chave publica ao ficheiro contrario.


#### Instruções de Uso

Inicialmente é necessário executar o script(rsagen.py) para geração dos pares de chaves:
$python3 rsagen.py

Para a sua utilização é so correr em terminais diferenter os seguintes comandos:
$python3 Server.py
$python3 Client.py


#### Ultima alteração em 14/12/2019