#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MB 2097152//524288 //512kb
#define MD 99999999
#define MC 256
#define EP 300

typedef struct T{
    unsigned char c[MC];
    unsigned int p;
}t;

typedef struct N{
    unsigned short d;
    struct N *z;
    struct N *o;
}n;

unsigned short k = 0;

unsigned int readbytes(short f, t *t, unsigned char *txt){
    unsigned int i = -1, n = read(f, txt, MB);
    k = 0;
    while (n != ++i){
        if(t[k].p < ++t[txt[i]].p){
            k = txt[i];
        }
    }
    k = t[k].p/255;
    return n;
}
void sort(unsigned char *s, t *t, unsigned short b){
    unsigned short a, i;
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
void shannon(unsigned char *s, t *t, unsigned short a, unsigned short b, int n, unsigned short m){
    if ((b - a) != 1){
        int u, p = 0, d1, d2 = MD;
        for (short i = a; i < b; i++){
            u = p;
            p = p + t[s[i]].p;
            d1 = abs(2*p-n);
            if (d1 < d2){
                d2 = d1;
                t[s[i]].c[m] = 1;
            }else{
		shannon( s, t, a, i, u, m+1);
                shannon( s, t, i, b, n-u, m+1);
                while(i < b){
                    t[s[i]].c[m] = 2;
                    ++i;
                }
                
            }
        }
    }
}
int payload(t *t, unsigned char *txt, unsigned char *p, unsigned int c){
    unsigned int b = 264, bk = 0, i = -1;
    unsigned short bt = 0, j = 0;
    p[256] = c; 
    p[257] = c>>8; 
    p[258] = c>>16; 
    p[259] = c>>24;

    while (c != ++i){
        while (MC != j){
            if (t[txt[i]].c[j]){
                p[b] = p[b] << 1 | t[txt[i]].c[j]>>1;
                ++bt;
                if (bt>>3){
                    bt = 0;
                    ++b;
                }
            }else
                break;

            ++j;
        }
        j=0;
    }
    if (!(bt>>3)){
        p[b] = p[b]<<(8-bt);
        b++;
    }
    bk = b - 264;
    p[260] = bk;
    p[261] = bk>>8;
    p[262] = bk>>16; 
    p[263] = bk>>24;
    return b;
}
unsigned short shorty(unsigned int n){
    unsigned short i = 0;
    while (n > i*k){
        ++i;
    }
    if (i > 255){
        i = 255;
    }
    return i;
}
float compMed(t *t, unsigned int *og, unsigned int total){
    float media = 0.0;
    unsigned short bit = 0, i = -1, f = -1;
    while (MC != ++i){
        if (og[i] != 0){
            bit=0;
            f = -1;
            while (MC != ++f){
                if (t[i].c[f] != 0)
                    bit++;
                else
                    break;
            }
            media = media + ((float)og[i]/total)*bit;
        }
    }
    return media;
}

float Log2n(float n){ 
    return (n > 1) ? 1 + Log2n(n / 2) : 0; 
}

float Entropy(unsigned int *og, unsigned int total){
    short i = -1;
    float entro = 0, prob = 0;
    while (MC!=i++){
        if (og[i]){
            prob = ((float)og[i]/total);
            entro = entro + prob*Log2n(1/prob);
        }
    }
    printf("\nEntropia do bloco %.3f\n", entro);
    return entro;
}
void printTable(t *t, unsigned char *s, unsigned int *og, int total){
    printf("\nSimb\tProb\tCodigo\n");
    for (int i = 0; i < MC; i++){
        printf("%x\t%.3f\t", s[i], (float)og[s[i]]/total);
        for (int j = 0; j < MC; j++){
            if (t[s[i]].c[j] != 0){
                printf("%d",t[s[i]].c[j]>>1);
            }else{
                printf("\n");
                break;
            }   
        }
    }
}
int zippar(short fd, short sd){
    t t[MC] = {0};
    unsigned char txt[MB], zip[MB<<1], s[MC];
    unsigned short o = 0, i = 0;
    unsigned int b = 0, nc = MB;

    while (nc == MB){     
        nc = readbytes(fd, t, txt);
        o = 0; i = 0;
        while (MC != i){
            s[i] = i;
            if (t[i].p){
                t[i].p = shorty(t[i].p);
                o = o + t[i].p;
            }            
            zip[i] = t[i].p;
            ++i;
        }
        sort(s, t, MC);
        shannon(s, t, 0, MC, o, 0);
        b = payload(t, txt, zip, nc);

        if (write(sd, zip, b)<0){
            perror("Erro na escrita do ficheiro compresso");    return 0;}
        memset(&t, 0, sizeof(t));
    }
    return 0;
}
int zipparcomlistagem(short fd, short sd){
    t table[MC] = {0};
    unsigned char txt[MB], zip[MB<<1], sorted[MC];
    unsigned short occurrencys = 0, i = 0, numBlock;
    unsigned int blockSize = 0, numChar = MB, original[MC], fileSize = 0, zipSize = 0;
    float average = 0, blockAverage = 0, entropy = 0;
    char op = 0;
    clock_t t;
    printf("\e[1;1H\e[2J");
    while (numChar == MB){
        numBlock++;
        printf("\n---------------------Bloco nr %d---------------------\n", numBlock);

        t = clock();
        numChar = readbytes(fd, table, txt);
        printf("\nTempo de leitura e contagem de ocorrencias = %.3fms\n", ((double)(clock()-t)/CLOCKS_PER_SEC)*1000);
        fileSize = fileSize + numChar;

        t = clock();
        occurrencys = 0; i = 0;
        while (MC != i){
            sorted[i] = i;
            if (table[i].p){
                original[i] = table[i].p;
                table[i].p = shorty(table[i].p);
                occurrencys = occurrencys + table[i].p;
            }            
            zip[i] = table[i].p;
            ++i;
        }
        printf("\nTempo de reducao e novo calculo de ocurrencias = %.3fms\n", ((double)(clock()-t)/CLOCKS_PER_SEC)*1000);
        
        entropy = entropy + Entropy(original,numChar);

        t = clock();
        sort(sorted, table, MC);
        printf("\nTempo de ordenacao = %.3fms\n", ((double)(clock()-t)/CLOCKS_PER_SEC)*1000);
        
        t = clock();
        shannon(sorted, table, 0, MC, occurrencys, 0);
        printf("\nTempo de Shannon-Fano = %.3fms\n", ((double)(clock()-t)/CLOCKS_PER_SEC)*1000);
          
        blockAverage = compMed(table, original, numChar);
        average = average + blockAverage;

        t = clock();
        blockSize = payload(table, txt, zip, numChar);
         if (write(sd, zip, blockSize)<0){
            perror("Erro na escrita do ficheiro compresso");    return 0;}

        printf("\nTempo de Construcao e escrita do payload = %.3fms\n", ((double)(clock()-t)/CLOCKS_PER_SEC)*1000);
        
        printf("\nComprimento medio do bloco = %.3f\n", blockAverage);

        printf("\nMostrar Tabela de Shannon-Fano?\n(s)im ou (n)ao\n");
        op = getchar();
        getchar();
        if (op == 's'){
            printTable(table, sorted, original, numChar);
        }  

        printf("\n(Pressione enter para continuar)");
        getchar();
        zipSize = zipSize + blockSize;
        memset(&table, 0, sizeof(table));
    }
    printf("\nReducao de %.3f%%\n", (float)100 - ((float)zipSize/fileSize)*100 );
    printf("\nComprimento medio do ficheiro = %.3f\n", average/numBlock);
    printf("\nEntropia do ficheiro = %.3f\n", entropy/numBlock);
    return 0;
}
struct N *insert(struct N *r, unsigned char d, unsigned char *c, unsigned short b){
    if (r == NULL){
        r = malloc(sizeof(struct N));
        r->d = EP;
        r->z = NULL;
        r->o = NULL;
    }
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
void decompress(t *t, struct N *r, unsigned char *c, unsigned char *d, unsigned int n){
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
int unzippar(short fd, short sd){
    t t[MC] = {0};
    n *r = NULL;

    unsigned char zip[MB<<1], txt[MB], s[MC];
    unsigned short o = 0, i = 0;
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
            ++i;}
        if (read(fd, zip, b)<0){
            perror("Erro na leitura do bloco compresso.");  return 0;}
        
        decompress(t,r, zip, txt, nc);

        if (write(sd, txt, nc)<0){
            perror("Erro na escrita do bloco original.");   return 0;}

        memset(&t, 0, sizeof(t));
        r = init(r);
    }
    return 0;
}
int main(int argc, char const *argv[]){
    clock_t tm = clock();
    short fd = 0, sd = 0;
    switch (argc){
        case 4://fast
            if ((fd = open(argv[2], O_RDONLY)) < 0){
                perror("Erro no terceiro argumento."); return 0;}

            if ((sd = open(argv[3], O_CREAT | O_WRONLY | O_TRUNC, 0666)) < 0){
                perror("Erro no quarto argumento."); return 0;}
            break;
        default:
            printf("Copyright (c) Hugo Machado 2019-2020 TSM\nCovid 19.0  Usage(linux only):\n(for a better and faster usage, compile with the next flags)\n\n");
            printf("->   gcc -o covid covid.c -O3 -march=native\n->   ./covid [-options] [file to compress/decompress] [name of result file]\n\nAvaible options:\n");
            printf("-l  verbose/debug mode(only compress)\n-c  compress mode\n-d  decompress mode\n");
            return 0;
    }

    switch (argv[1][1]){
        case 'c':
            zippar(fd, sd);
            printf("Tempo de execucao = %.3fms\n", ((double)(clock()-tm)/CLOCKS_PER_SEC)*1000);
            break;
        case 'd':
            unzippar(fd, sd);
            printf("Tempo de execucao = %.3fms\n", ((double)(clock()-tm)/CLOCKS_PER_SEC)*1000);
            break;
        case 'l':
            zipparcomlistagem(fd, sd);
            break;
        default:
            printf("Run ./covid to see the instructions\n");
            break;
    }
    close(fd);  close(sd);
    return 0;
}
