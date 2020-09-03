Guião 5
Comunicação entre cliente-servidor

As scripts Client.py e Server.py constituem uma implementação muito 
básica de uma aplicação que permite a um número arbitrário de clientes 
comunicar com um servidor que escuta num dado port (e.g. 8888). 
O servidor atribui um número de ordem a cada cliente, e simplesmente 
faz o dump do texto enviado por eese cliente (prefixando cada linha 
com o respectivo número de ordem). Quando um cliente fecha a ligação, 
o servidor assinala o facto.

Neste Guião utilizamos o método de encriptação AES com CBC, o que implica
exista padding, o método de padding utilizado foi o PKCS7, e utilizamos
o HMAC para confirmar a integridade e autenticidade da mensagem.

Instruçoes de uso.
É necessário correr o servidor antes do cliente para o seu funcionamento.

Observação:
Por alguma razão, a função de verify() do HMAC, não está a funcionar, para
dar a volta ao problema, fizemos uma simples comparação da tag da mensagem
com a função finalize().Temos a completa noção que esta solução não é viavel.