1-gera se um par de chaves publica e privada para o servidor e para o cliente, atraves do rsa (rsagen.py).
2-o cliente gera a sua chave privada de diffie, atraves dela a publica e envia para o servidor.
3-o servidor tb gera a sua chave privada e publica de diffie, e com a chave privada de rsa assina as duas chaves publicas de diffie.
4-envia para o cliente a sua chave publica e assinatura da chaves, o cliente verifica a assinatura com a chave publica rsa do servidor.
5-depois o cliente com a sua chave privada rsa assina as duas chave publicas de diffie, e envia a assinatura para o servidor.
6-o servidor com a chave publica rsa do cliente verifica a assinatura.
7-depois é feito o exchange de chaves publicas para derivar a chave para a encriptação. 