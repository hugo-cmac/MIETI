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

int calcNumBlocks(int s){
    if (s < MAXBUFFER){
        return 1;
    }else{
        return (s/MAXBUFFER + 1); 
    }
}

int calcNumChars(int fd, table *t, char *text){
    int n = read(fd, text, MAXBUFFER);
    for (size_t i = 0; i < n; i++){
        t[text[i]].prob++;
    }
    return n;
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

int shorten(table *t){
    int biggest = 0, n = 0, sum = 0;
    for (int i = 1; i < MAXCHAR; i++){
        if (t[biggest].prob < t[i].prob){
            biggest = i;
        }   
    }
    while ((t[biggest].prob>>n) > MAXCHAR-1){
        n++;
    }
    for (size_t i = 0; i < MAXCHAR; i++){
        t[i].prob = t[i].prob >> n;
        sum = sum + t[i].prob;
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


void headMaker(table *t, int bs, unsigned char *h){
    intTOchar c;    c.n = bs;
    for (int i = 0; i < MAXCHAR; i++){
        h[i] = (int)t[i].prob;
        if (i==255){
            for (int j = 1; j < 5; j++){
                h[i+j] = c.w[j-1];
            }
        }
    }
}

int payloadMaker(table *t, char *text, char *p, int bs){
    int byte = 264, bit = 0;
    for (int i = 0; i < bs; i++){
        for (int j = 0; j < MAXCHAR; j++){
            if (t[text[i]].code[j]!=0){
                p[byte] = p[byte] << 1 | t[text[i]].code[j]>>1;
                bit++;
                if (bit>>3){
                    bit = 0;
                    byte++;
                }
            }else{
                break;
            }
        }
    }
    intTOchar c; c.n = byte-264;
    for (int i = 0; i < 4; i++){
        p[260+i] = c.w[i];
    }
    return (byte);
}

int main(int argc, char const *argv[]){
    clock_t time = clock();
    if (argc<2){
        perror("Falta indicar o ficheiro para compressao"); return 0;
    }
    if (argc<3){
        perror("Falta indicar o nome do ficheiro compresso"); return 0;
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd<0){
        perror("Nao foi possivel abrir o ficheiro"); return 0;
    }
    int sd = open(argv[2], O_CREAT | O_WRONLY, 0666);
    if (sd<0){
        perror("erro"); return 0;
    }
    int fileSize = lseek( fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    int blocks = calcNumBlocks(fileSize);
    
    table t[MAXCHAR] = {0};
    char *text = (char *)malloc(MAXBUFFER), *compBlock;
    int sorted[MAXCHAR] = {0};
    while (blocks != 0){
        int blockSize = calcNumChars(fd, t, text);
        int occurrencys = shorten(t);
        
        sort( sorted, t, MAXCHAR);
        //printf("Tempo de LEITURA E SORT = %fms\n", ((float)(clock()-time)/CLOCKS_PER_SEC)*1000);
        
        shannon(sorted, t, 0, MAXCHAR, occurrencys, 0);
        //printf("Tempo de SHANNON = %fms\n", ((float)(clock()-time)/CLOCKS_PER_SEC)*1000);
      
        compBlock = (char *)malloc(blockSize);
        headMaker(t, blockSize, compBlock);
        //printf("Tempo de header = %fms\n", ((float)(clock()-time)/CLOCKS_PER_SEC)*1000);

        int compSize = payloadMaker(t, text, compBlock, blockSize);
        //printf("Tempo de payload = %fms\n", ((float)(clock()-time)/CLOCKS_PER_SEC)*1000);
        write(sd, compBlock, compSize);

        free(compBlock);
        memset(&t, 0, sizeof(t));
        blocks--;
    }
    free(text);
    close(fd);  close(sd);
    printf("Tempo de execucao = %fms\n", ((float)(clock()-time)/CLOCKS_PER_SEC)*1000);
    return 0;
}
