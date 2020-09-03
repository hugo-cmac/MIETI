# Versão final da aplicação Cliente / Servidor

Como melhoramento ao pedido nos guiões, sugere-se a geração dos certificados utilizados pela aplicação. Para tal pode usar qualquer das várias sugestões de software livre disponívies para o efeito - uma escolha óbvia é naturalmente o openssl a que já recorremos antes, mas preferindo podem utilizar uma solução de mais alto nível, como a EJBCA.


#### Objectivos 

Para este guião, foi nos proposto que criasse-mos os nossos certificados para aplicação.
Na aplicação em si as únicas alterações feitas foi alteração dos tamanhos da assinatura, de resto manteve-se com a mesma estrutura.
Para criação de certificados desenvolvemos um script á parte para os criar.


#### Opções tomadas e técnicas utilizadas

As técnicas utilizadas foram as mesmas dos guiões anteriores, de cliente/servidor: 
-Protocolo Diffie Hellman(geração de um segredo partilhado); 
-Os pares de chaves publica/privada dos certificados utilizamos o RSA, combinados com certificados de chave pública X509;
-O objeto PKCS12 para guardar chaves privadas e certificados numa Keystore.

Neste último guião decidimos usar a biblioteca PyOpenSSL para criar os certifacados.
Para este efeito, começamos por criar o nosso CA(EC_self_signed.cer), um certificado auto-assinado, este é semelhante ao CA fornecido pelo docente para o Guião anterior.
E com a chave privada do nosso CA permitiu-nos assinar os certificados do servidor e cliente.
Guarda-mos os certificados e chaves privadas, do servidor e cliente, em PKCS12(numa keystore).

Obs: Validade escolhida para os certificados é de apenas um dia.


#### Instruções de Uso

Em primeiro lugar devemos correr o gerador de certificados, com o comando:
$python3 buildcert.py

Depois de o certificados serem criados, é so correr em terminais diferenter os seguintes comandos:
$python3 Server.py
$python3 Client.py


#### Ultima alteração em 14/12/2019