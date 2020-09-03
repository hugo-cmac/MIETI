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

#define BUFSIZE 1024
#define MAXDIF  99999999

typedef struct Tabela{
    unsigned char simbolo;
    int probabilidade;
    char codigo[9];
}tabela;

tabela table[256];
int nEntradas = 0;

void insert(int index, unsigned char byte){
    table[index].simbolo = byte;
    table[index].probabilidade++;
    nEntradas++;
}

void searchNsum(unsigned char byte){
    if (nEntradas == 0){
        insert(0, byte);
        return;
    }else{
        int i;
        for (i = 0; i < nEntradas; i++){
            if (byte == table[i].simbolo){
                table[i].probabilidade++;
                return;
            }
        }
        insert(i, byte);
    }
}

void byteDiv(int fd){
    unsigned char *byte = (unsigned char*)malloc(524288);
    int n;
    while((n = read(fd, byte, 524288)) != 0){
        for (int i = 0; i < n; i++){
            searchNsum(byte[i]);
        }
    }
    free(byte);
}

void swap(int i){
    tabela temp = table[i+1];
    table[i+1] = table[i];
    table[i] = temp;
}


void Bubblesort(int last){
    int swaped, i;
    do{
        swaped = 0;
        for (i = 0; i < last-1; i++){
            if(table[i].probabilidade < table[i+1].probabilidade){
                swap(i);
                swaped = 1;
            }
        }
        last = i;
    } while (swaped);
}

int abs(int n){
    if(n < 0)
        n = -n;
    return n;
}

void shannon(int start, int stop, int nr, int nbit){
    if ((stop - start) != 1){
        int u, p = 0, dif1, dif2 = MAXDIF;
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

float prob(int n, int s){
    return (float)(((float)n)/s);
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
    byteDiv(fd);
    Bubblesort(nEntradas);
    printf("\nTempo de LEITURA E SORT = %fms\n", ((float)(clock()-t)/CLOCKS_PER_SEC)*1000);

    shannon(0, nEntradas, size, 0);
    printf("\nTempo de SHANNO = %fms\n", ((float)(clock()-t)/CLOCKS_PER_SEC)*1000);
    //printf("| SIMBOLO | PROBABILIDADE | CODIGO BINARIO |\n");
    //for (size_t i = 0; i < nEntradas; i++){
    //    printf("|    %x   |    %f   |    %s    |\n", table[i].simbolo, prob( table[i].probabilidade, size), table[i].codigo);
    //}
    printf("\nTempo de execucao = %fms\n", ((float)(clock()-t)/CLOCKS_PER_SEC)*1000);
    return 0;
}
