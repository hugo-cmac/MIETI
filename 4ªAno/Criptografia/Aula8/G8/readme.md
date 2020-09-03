# Guião 8: Protocolo StS usando certificados

No guião desta semana vamos concluir a implementação do protocolo Station_to_Station fazendo uso de certificados X509. Para tal vamos incorporar a funcionalidade explorada no último guião (validação dos certificados), por forma a assegurar a cada um dos intervenientes que fazem uso da chave pública correcta na verificação da assinatura.

Concretamente, o protocolo a implementar irá ser então:

    1.Alice → Bob   :   gx
    2.Bob   → Alice :   gy, SigB(gx, gy), CertB
    3.Alice → Bob   :   SigA(gx, gy), CertA
    4.Alice,  Bob   :   K = g(x*y)

Como ponto de partida, disponibiliza-se:

    1.Uma keystore PKCS12 contendo o Certificado (e respectiva chave privada) para o rervidor: Servidor.p12
    2.Uma keystore PKCS12 contendo o Certificado (e respectiva chave privada) para o cliente: Cliente1.p12
    3.O Certificado (em formato DER) da CA utilizada: CA.cer


#### Objectivos 

Para este guião, foi nos proposto que integrasse-mos os certificados de chave publica X509, com o guião anterior com o propósito de estabelecer a relação de confiança entre identidade e chave pública. Para que a comunicação neste canal público seja segura.


#### Opções tomadas e técnicas utilizadas

As técnicas utilizadas foram as mesmas do guião anterior, com a utilização de certificados:

-Protocolo Diffie Hellman(geração de um segredo partilhado); 
-Certificados de chave pública X509, com chave publica RSA;
-O objeto PKCS12 para extrair os certificados e chaves privadas das keystores fornecidas.Estas chaves privadas(do par de chaves RSA) serão utilizadas para a assintura.

Neste guião usamos o objeto crypto da biblioteca do PyOpenSSL, para os metodos de leitura da keystore, os metodos load.. e dump.. para ler-mos ou converter os certificados em determinados formatos.
Do objeto crypto utilizado tambem o objeto X509Store() e X509StoreContext(), fazer a verificação da cadeia de certificados.


#### Instruções de Uso

Para a sua utilização é so correr em terminais diferenter os seguintes comandos:
$python3 Server.py
$python3 Client.py


#### Ultima alteração em 14/12/2019