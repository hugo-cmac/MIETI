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
    struct Tabela *next;
}tabela;

tabela *cabeca;

tabela* insert(unsigned char byte){
    tabela *temp = (tabela*) malloc(sizeof(tabela));
    temp->simbolo = byte;
    temp->probabilidade++;
    return temp;
}

void searchNsum(unsigned char byte){
    if (cabeca == NULL){
        cabeca = insert(byte);
    }else{
        tabela *now = cabeca;
        while (now->simbolo != byte){
            if (now->next == NULL){
                now->next = insert(byte);
                return;
            }else{
                now = now->next;
            }
        }
        now->probabilidade++;
    }
}

void calcProbs(int fd){
    unsigned char *byte = (unsigned char*)malloc(BUFSIZE);
    int n;
    while((n = read(fd, byte, BUFSIZE)) != 0){
        for (int i = 0; i < n; i++){
            searchNsum(byte[i]);
        }
    }
    free(byte);
}

void swap(tabela *a, tabela *b) { 
    unsigned char simb = a->simbolo;
    int prob = a->probabilidade; 
    a->simbolo = b->simbolo;
    a->probabilidade = b->probabilidade;
    b->simbolo = simb;
    b->probabilidade = prob;
} 

void printlista(tabela *n){
    while (n != NULL){
        printf("%c -- %d\n", n->simbolo, n->probabilidade);
        n = n->next;
    }
}

void sort(tabela *start) { 
    int swapped;
    tabela *now, *last = NULL; 
    do{
        swapped = 0;
        now = start;
        while (now->next != last){
            if (now->probabilidade < now->next->probabilidade){
                swap(now, now->next); 
                swapped = 1; 
            } 
            now = now->next; 
        } 
        last = now; 
    } while (swapped); 
} 

float prob(int n, int s){
    return (float)(((float)n)/s);
}

int abs(int n){
    if(n < 0)
        n = -n;
    return n;
}

void shannon(tabela *start, tabela *end, int nr, int nbit){
    if (start->next != end){
        tabela *now = start;
        int u, p = 0, dif1, dif2 = MAXDIF;
        while (now != end){
            u = p;
            p = p + now->probabilidade;
            dif1 = abs(p-(nr-p));
            //printf("%d < %d \n", dif1, dif2);
            if (dif1 < dif2){
                dif2 = dif1;
                now->codigo[nbit] = '0';
            }else{
                printf("%d\n", nbit);
                shannon(start, now, u, nbit+1);
                shannon(now, end, nr-u, nbit+1);            
                while (now->next != end){
                    now->codigo[nbit] = '1';
                    now = now->next;
                }
                if (now->next != NULL){
                    now->next->codigo[nbit] = '1';
                }
            }
            //printf("%x --\n", now->simbolo);
            now = now->next;
        }
    }
}

/*void shannon(int start, int stop, int nr, int nbit){
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
}*/

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
    
    calcProbs(fd);

    tabela *now = cabeca;
    sort(cabeca);
    shannon(cabeca, NULL, size, 0);
    printf("| SIMBOLO | PROBABILIDADE | CODIGO BINARIO |\n");
    now = cabeca;
    while (now != NULL){
        printf("| \t%x\t | \t%f\t | \t%s\t |\n", now->simbolo, prob( now->probabilidade, size), now->codigo);
        now = now->next;
    }
    printf("\nTempo de execucao = %fms\n", ((float)(clock()-t)/CLOCKS_PER_SEC)*1000);
    return 0;
}
