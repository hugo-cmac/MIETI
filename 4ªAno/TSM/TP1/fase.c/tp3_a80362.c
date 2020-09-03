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

#define MAXBUFFER 524288 //512kb
#define MAXDIF 99999999
#define MAXCHAR 256

typedef union Convert{
    int n;
    char w[4];
}intTOchar;

typedef struct Table{
    unsigned int prob;
    unsigned int code[MAXCHAR];
}table;

typedef struct Stack{
    int start; 
    int stop; 
    int nr;
    int nbit;
    struct Stack *previous;
}stack;

int blockSize = 0;


int getHeader(table *t, unsigned char *h){
    intTOchar c;
    int bs;
    for (int i = 0; i < MAXCHAR; i++){
        t[i].prob = h[i];
        if (i==255){
            for (int j = 1; j < 5; j++){
                c.w[j-1] = h[i+j];
            }
            bs = c.n;
            for (int j = 5; j < 9; j++){
                c.w[j-5] = h[i+j];
            }
            blockSize = c.n;
        }
    }
    return bs;
}

void sort(int *s, table *t, int size){
    for (int i = 0; i < MAXCHAR; i++){
        s[i] = i; 
    } 
    int swaped = 1, i;
    do{
        swaped = 0;
        for (i = 0; i < size -1; i++){
            if(t[s[i]].prob < t[s[i+1]].prob){
                int temp = s[i+1];
                s[i+1] = s[i];
                s[i] = temp;
                swaped = 1;
            }
        }
        size = i;
    } while (swaped);
}

int calcProbs(table *t, int *s){
    int sum = 0;
    for (int i = 0; i < MAXCHAR; i++){
        sum = sum + t[s[i]].prob;
    }
    return sum;
}

int abs(int n){
    if(n < 0)    n = -n;
    return n;
}

void shannon(int *s, table *t, int start, int stop, int nr, int nbit){
    if ((stop - start) != 1){
        int u, p = 0, dif1, dif2 = MAXDIF;
        for (int i = start; i < stop; i++){
            u = p;
            p = p + t[s[i]].prob;
            dif1 = abs(2*p-nr);
            if (dif1 < dif2){
                dif2 = dif1;
                t[s[i]].code[nbit] = 1;
            }else{
                shannon( s, t, start, i, u, nbit+1);
                shannon( s, t, i, stop, nr-u, nbit+1);
                while(i < stop){
                    t[s[i]].code[nbit] = 2;
                    i++;
                }
            }
        }
    }
}
void decompress(table *t, int *s, unsigned char *c, unsigned char *d, int nChar){
    int search = 0, nbit = 0;
    int bit = 0, ch = 0, byte = 0;

    while (ch != nChar){
        for (int i = 0; i < 8; ++i){
            bit = (c[byte] >> (7-i)) & 1;
            for (int j = search; j < MAXCHAR; ++j){
                if (bit == (t[s[j]].code[nbit]>>1)){
                    if (t[s[j]].code[nbit+1] != 0){
                        search = j;
                        ++nbit;
                    }else{
                        search = 0;
                        d[ch] = s[j];
                        ++ch;
                        nbit = 0;
                        if (ch == nChar){  break;}
                    }
                    break;
                }       
            }
        }
        ++byte;
    }
}

int main(int argc, char const *argv[]){
    clock_t time = clock();
    if (argc<2){
        perror("Falta indicar o ficheiro para descompressao"); return 0;
    }
    if (argc<3){
        perror("Falta indicar o nome do ficheiro destino"); return 0;
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd<0){
        perror("Nao foi possivel abrir o ficheiro"); return 0;
    }
    int sd = open(argv[2], O_CREAT | O_WRONLY, 0666);
    if (sd<0){
        perror("erro"); return 0;
    }

    table t[MAXCHAR] = {0};
    int sorted[MAXCHAR] = {0};
    unsigned char *text = (char *)malloc(MAXBUFFER);
    unsigned char *decompressed = (char *)malloc(MAXBUFFER);

    while (read(fd, text, 264)){
        int nChar = getHeader(t, text);
        //printf("GetHeader = %fms\n", ((float)(clock()-time)/CLOCKS_PER_SEC)*1000);
        sort( sorted, t, MAXCHAR);
        //printf("sort = %fms\n", ((float)(clock()-time)/CLOCKS_PER_SEC)*1000);
        int occurrencys = calcProbs(t, sorted);
        //printf("ocorrencias = %fms\n", ((float)(clock()-time)/CLOCKS_PER_SEC)*1000);
        shannon(sorted, t, 0, MAXCHAR, occurrencys, 0);
        //printf("shanon = %fms\n", ((float)(clock()-time)/CLOCKS_PER_SEC)*1000);
        read(fd, text, blockSize);
        decompress(t, sorted, text, decompressed, nChar);
        printf("descompressao = %fms\n", ((float)(clock()-time)/CLOCKS_PER_SEC)*1000);
        exit(0);
        write(sd, decompressed, nChar);
        memset(&t, 0, sizeof(t));
    }
    free(text);
    free(decompressed);
    close(fd);  close(sd);
    printf("Tempo de execucao = %fms\n", ((float)(clock()-time)/CLOCKS_PER_SEC)*1000);
    return 0;
}
