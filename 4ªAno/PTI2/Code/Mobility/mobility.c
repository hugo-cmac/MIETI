#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#define ESQUERDA 0
#define FRENTE   1
#define DIREITA  2
#define TRAS     3

int func(int node, int f){
    char *mob[4][5] = {
        {" setdest 675.0 40.0 40.0\"", " setdest 925.0 40.0 40.0\"", " setdest 1165.0 40.0 40.0\"", " setdest 1405.0 40.0 40.0\"", " setdest 1645.0 40.0 40.0\""},
        {" setdest 675.0 280.0 40.0\"", " setdest 925.0 280.0 40.0\"", " setdest 1165.0 280.0 40.0\"", " setdest 1405.0 280.0 40.0\"", " setdest 1645.0 280.0 40.0\""},
        {" setdest 675.0 520.0 40.0\"", " setdest 925.0 520.0 40.0\"", " setdest 1165.0 520.0 40.0\"", " setdest 1405.0 520.0 40.0\"", " setdest 1645.0 520.0 40.0\""},
        {" setdest 675.0 795.0 40.0\"", " setdest 925.0 795.0 40.0\"", " setdest 1165.0 795.0 40.0\"", " setdest 1405.0 795.0 40.0\"", " setdest 1645.0 795.0 40.0\""},
    };

    int next = 0, anterior = -1;
    int linha = 3, coluna = 0;
    int tempo = 10, passou = 1;
    char *temp = malloc(125);
    sprintf(temp, "\n\n$node_(%d) set X_25.0\n$node_(%d) set Y_795.0\n$node_(%d) set Z_0.00\n\n$ns_ at 1.00 \"$node_(%d) setdest 675.0 795.0 80.0\"\n", node, node, node, node);
    write(f, temp, strlen(temp));

    for (size_t i = 0; i < 20; i++){
        next = rand()%(4);
        switch (next){
            case ESQUERDA:
                if (coluna - 1 > -1 && anterior != DIREITA)
                    coluna--;
                else
                    passou = 0;
                break;
            case FRENTE:
                if (linha - 1 > -1 && anterior != TRAS)
                    linha--;
                else
                    passou = 0;
                break;
            case DIREITA:
                if (coluna + 1 < 5 && anterior != ESQUERDA)
                    coluna++;
                else
                    passou = 0;
                break;
            case TRAS:
                if (linha + 1 < 4 && anterior != FRENTE)
                    linha++;
                else
                    passou = 0;
                break;
        }
        if (passou){
            sprintf(temp,"$ns_ at %d.00 \"$node_(%d)%s\n", tempo, node, mob[linha][coluna]);
            write(f, temp, strlen(temp));
            anterior = next;
            tempo = tempo + 8;
        }else{
            i--;
            passou = 1;
        }
    }
    return 0;
}

int main(int argc, char const *argv[]){
    srand((unsigned)time(NULL));

    //int f = open("vmw.scen", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    //func(25, f);
    //func(39, f);
    //close(f);


    //f = open("volks.scen", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    //func(30, f);
    //func(40, f);
    //close(f);

    int f = open("alert.scen", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    //func(18, f);
    //func(34, f);
    func(35, f);
    //func(36, f);
    close(f);

    return 0;
}
