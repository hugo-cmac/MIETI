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

#define MB 2097152//524288 //512kb
#define MD 99999999
#define MC 256
#define EP 300

//gcc -o decoder decoder.c -O3 -march=native

typedef struct{
    unsigned char c[MC];
    unsigned int p;
}t;
typedef struct N{
    unsigned short d;
    struct N *z;
    struct N *o;
}n;
void sort(unsigned short *s, t *t, unsigned short b){
    unsigned short a = 1, i;
    do{
        a = 0;
        i = 0;
        while (i != (b-1)){
            if(t[s[i]].p < t[s[i+1]].p){
                s[i+1] = s[i+1] ^ s[i];
                s[i] = s[i+1] ^ s[i];
                s[i+1] = s[i+1] ^ s[i];
                a = 1;
            }
            ++i;
        }
        b = i;
    } while (a);
}
int abs(int n){
    if(n < 0)    n = -n;
    return n;
}
void shannon(unsigned short *s, t *t, unsigned short a, unsigned short b, int n, unsigned short m){
    if ((b - a) != 1){
        int u, p = 0, d1, d2 = MD;
        unsigned short f;
        for (short i = a; i < b; i++){
            u = p;
            p = p + t[s[i]].p;
            d1 = abs(2*p-n);
            if (d1 < d2){
                d2 = d1;
                t[s[i]].c[m] = 1;
            }else{
                f = i;
                while(i < b){
                    t[s[i]].c[m] = 2;
                    i++;
                }
                shannon( s, t, a, f, u, m+1);
                shannon( s, t, f, b, n-u, m+1);
            }
        }
    }
}
struct N *createNode(){
    struct N *n = malloc(sizeof(struct N));
    n->d = EP;
    n->z = NULL;
    n->o = NULL;
    return n;
}
struct N *insert(struct N *r, unsigned char d, unsigned char *c, unsigned short b){
    if (r == NULL)
        r = createNode();

    switch (c[b]){
        case 0:
            r->d = d;
            break;
        case 1:
            r->z = insert(r->z, d, c, b+1); 
            break;
        case 2:
            r->o = insert(r->o, d, c, b+1); 
            break;
    }
    return r;
}
struct N *init(struct N *r){ 
    if (r->d == EP){
        r->z = init(r->z); 
        r->o = init(r->o);
    }else{
        r->d = EP;
    }
    return r;
}
void decompress(t *t, struct N *r, unsigned short *s, unsigned char *c, unsigned char *d, unsigned int n){
    unsigned int ch = 0, b = 0;
    unsigned short bt = 0, i = 0;
    struct N *tp = r;

    while (n != ch){
        for (i = 0; i < 8; ++i){
            bt = (c[b] >> (7-i)) & 1;
            if (bt)
                tp = tp->o;
            else
                tp = tp->z;
                
            if (EP ^ tp->d){
                d[ch] = tp->d;
                tp = r;
                if (n == ++ch)
                    break;
            }
        }
        ++b;
    }
}
void printTable(t *t, unsigned short *s){
    for (int i = 0; i < MC; i++){
        printf("%x -> ", s[i]);
        for (int j = 0; j < MC; j++){
            if (t[s[i]].c[j] != 0){
                printf("%d",t[s[i]].c[j]);
            }else{
                printf("\n");
                break;
            }   
        }
    }
        
}
int main(int argc, char const *argv[]){
    clock_t tm = clock();
    short fd = 0, sd = 0;
    switch (argc){
        case 1:
            perror("Falta indicar o ficheiro para descompressao."); return 0;
            break;
        case 2:
            perror("Falta indicar o nome do ficheiro destino."); return 0;
            break;
        case 3:
            if ((fd = open(argv[1], O_RDONLY)) < 0){
                perror("Nao foi possivel abrir o primeiro ficheiro."); return 0;}

            if ((sd = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0666)) < 0){
                perror("Nao foi possivel abrir o segundo ficheiro."); return 0;}
            break;
        default:
            perror("Argumentos  Invalidos."); return 0;
            break;
    }

    t t[MC] = {0};
    unsigned char zip[MB<<1], txt[MB];

    struct N *r = NULL;
    unsigned short s[MC] = {0}, o = 0, i = 0;
    unsigned int nc = 0, b = 0;

    while (read(fd, zip, 264)){
        o = 0; i = 0;

        while (MC != i){
            s[i] = i;
            t[i].p = zip[i];
            o = o + t[i].p;
            ++i;
        }

        nc = zip[256]; nc += zip[257]<<8; nc += zip[258]<<16; nc += zip[259]<<24;
        b  = zip[260]; b  += zip[261]<<8; b  += zip[262]<<16; b  += zip[263]<<24;
        
        sort( s, t, MC);
        shannon(s, t, 0, MC, o, 0);
        
        i=0;
        while (MC != i){
            r = insert(r, s[i], t[s[i]].c, 0);
            ++i;
        }
       
        
        if (read(fd, zip, b) < b){
            perror("aqui");
        }
        
        decompress(t,r, s, zip, txt, nc);
        
        write(sd, txt, nc);

        memset(&t, 0, sizeof(t));
        r = init(r);
    }
    close(fd);  close(sd);
    printf("Tempo de execucao = %fms\n", ((double)(clock()-tm)/CLOCKS_PER_SEC)*1000);
    return 0;
}