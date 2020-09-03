#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

typedef struct Tabela{
    unsigned char simbolo;
    int probabilidade;
    char codigo[9];
}tabela;

tabela table[256];
int nEntradas = 0;

int searchNsum(unsigned char byte){
    if (nEntradas == 0){
        table[0].simbolo = byte;
        table[0].probabilidade++;
        nEntradas++;
        return 0;
    }else{
        int i;
        for (i = 0; i < nEntradas; i++){
            if (byte == table[i].simbolo){
                table[i].probabilidade++;
                return 0;
            }
        }
        nEntradas++;
        table[i].simbolo = byte;
        table[i].probabilidade++;
    }
    return 0;
}

void sort(int n){
    for(int x = 0; x < n; x++){
        for(int y = 0; y < n-1; y++){
            if(table[y].probabilidade < table[y+1].probabilidade){
                tabela temp = table[y+1];
                table[y+1] = table[y];
                table[y] = temp;
            }
        }
    }
}

void calcProbs(int fd){
    unsigned char byte;
    while(read(fd, &byte, 1) != 0){
        searchNsum(byte);
    }
}

float prob(int n, int s){
    return (float)(((float)n)/s);
}

int abs(int n){
    if(n < 0)
        n = -n;
    return n;
}

void shannon(int start, int stop, int nr, int nbit){
    if ((stop - start) != 1){
        int u, p = 0, dif1, dif2 = 99999999;
        for (int i = start; i < stop; i++){
            u = p;
            p = p + table[i].probabilidade;
            dif1 = abs(p-(nr-p));
            if (dif1 < dif2){
                dif2 = dif1;
                table[i].codigo[nbit] = '0';
            }else{
                shannon(start, i, u, nbit+1);
                shannon(i, stop, nr-u, nbit+1);
                while(i < stop){
                    table[i].codigo[nbit] = '1';
                    i++;
                }
            }
        }
    }
}

int main(int argc, char const *argv[]){
    clock_t t = clock();
    if (argc<2){
        perror("file"); return 0;
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd<0){
        perror("file"); return 0;
    }
    int size = lseek( fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    for (size_t i = 0; i < 256; i++){    strcpy(table[i].codigo,"        ");}
    calcProbs(fd);
    sort(nEntradas);
    shannon(0, nEntradas, size, 0);
    printf("| SIMBOLO | PROBABILIDADE | CODIGO BINARIO |\n");
    for (size_t i = 0; i < nEntradas; i++){
        printf("|    %x   |    %f   |    %s    |\n", table[i].simbolo, prob( table[i].probabilidade, size), table[i].codigo);
    }
    printf("\nTempo de execucao = %fs\n", (float)(clock()-t)/CLOCKS_PER_SEC);
    return 0;
}
