IND-CPA-chacha20
Nonce diferente em cada cifra, portanto inquebrável,nenhum resultados vai
semelhanca ao anterior, 50% de probabilidade.
limitamos nos a comparar os dois criptogramas, sem critério de comparação.

IND-CPA-EncThenMac
Nonce diferente em cada cifra, mas como o tag calculado pelo HMAC não é
encriptado, é esta vulnerabilidade que se torna o nosso critério de comparação
e assim torna-se quebrável.100% de probabilidade.

IND-CPA-CB
Neste usamos como metodo de encriptação o AES, cifra de blocos, e como temos 
uma mensagem onde os primeiros 16bytes são iguais aos segundos 16bytes, usamos
isso como criterio de comparação, desta forma temos 100% de probabilidade de 
acertar.

