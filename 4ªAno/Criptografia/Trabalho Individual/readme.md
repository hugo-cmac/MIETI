Funcionamento de uma curva elitica em Criptografia

As curvas eliticas são, na sua forma mais elementar, um tipo de curvas que satisfaz um tipo de requisitos matemáticos.

A sua é a seguinte:

        y^2 = x^3 + A*x + B

    Os valores A e B têm de cumprir os seguintes requisitos:

        4*A^3 + 27*B^2 != 0

Para gerar uma curva elitica é necessário escolher os pontos A e B, que são os pontos que definem a curva. 
Através destes parametros é gerado um ponto gerador(P), este é um ponto da curva.
Depois é escolhido um ponto da curva aleatóriamente interpretado por Q.
Se traçamos uma reta nesses 2 pontos obtemos um ponto interpretado com -R e se traçar mos um linha vertical nesse ponto -R, vai intercetar a reta noutro ponto interpretado por R.
Este ponto R é igual á soma de P + Q.

Se o ponto P e o Q forem o mesmo, é traçada uma linha tangente á curva no ponto P, e o segundo ponto obtido através dessa linha vai ser o ponto -R

A eficiência das curvas eliticas vem através

Utilização da curva elitica com o Protocolo de Diffie-Hellman

Nesta utilização o ponto gerador é o G que é devolvido através da inicialização da curva, com os respectivos parâmetros.

Depois o utlizador escolhe um número secreto, preferivelmente de forma aleatória, dentro do intervalo:

                1 <= número secreto(a) <= (N - 1)
    
    N: é um número primo de ordem a G.

A multiplicação do ponto gerador com esse número dá origem á chave pública:
            número secreto(a) * G = A
        Esta chave pública corresponde a um ponto da curva, onde ficaria o ponto R.

Esta chave pública multiplicada com o número secreto da outra entidade envólvida, gera a chave partilhada:
            número secreto(a) * B = chave partilhada


https://www.allaboutcircuits.com/technical-articles/elliptic-curve-cryptography-in-embedded-systems/
https://scipython.com/blog/the-double-pendulum/