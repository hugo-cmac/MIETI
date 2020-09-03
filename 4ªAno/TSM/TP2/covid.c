#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MB 2097152//524288 //512kb
#define M  2110000
#define MD 99999999
#define MC 256
#define EP 300
#define SP 0xfd

typedef struct T{
    unsigned char c[MC];
    unsigned int p;
}t;

typedef struct N{
    unsigned short d;
    struct N *z;
    struct N *o;
}n;

typedef struct D{
    unsigned short d;
    unsigned short n;
}d;

unsigned short bit = 0;

unsigned short readLZW(unsigned char *zip, unsigned int *pos){
    unsigned short c = 0;
    if (bit){
        c = zip[*pos]<<4 ^ zip[*pos+1]>>4;
        *pos = *pos + 1;
        bit = 0;
    }else{
        c = (zip[*pos]&0x0f)<<8 ^ zip[*pos+1];
        *pos = *pos + 2;
        bit = 1;
    }
    return c;
}

unsigned int writeLZW(unsigned char *zip, unsigned int s, unsigned short c){
    if (bit){
        zip[s] = c >> 4;
        zip[++s] = 0x0f & c;
        bit = 0;
    }else{
        zip[s] = zip[s] << 4 | c >> 8;
        zip[++s] = c;
        s++; bit = 1;
    }
    return s;
}

d putValue(unsigned short v, unsigned short l){
    d t;
    t.d = v;
    t.n = l;
    return t;
}

unsigned int lzw_c(unsigned char *txt, unsigned char *lzw, unsigned int n){
    d d[256][4000] = {0};
    unsigned s = 0, i = 0;
    unsigned short l = 0, b = 0, a = 0, c = MC;
    bit = 1;
    while (i < n){
        while ((i < n) && (c != 4096)){
            if(d[txt[i]][a].n == 0)
                d[txt[i]][a] = putValue(txt[i],++l);        
           
            a = d[txt[i]][a].n;
            while ((d[txt[++i]][a].n != 0) && (i < n)){
                b = a;
                a = d[txt[i]][a].n;
            }
            s = writeLZW(lzw, s, d[txt[i-1]][b].d);
            d[txt[i]][a] = putValue(c++, ++l);
            b = a = 0;
        }

        l = 0; c = MC;
        memset(&d, 0, sizeof(d));
        if (s>i)
            return 0;
    }
    if (!bit){
        lzw[s] = lzw[s] << 4;
        s++;
    }
    return s;
}

unsigned int lzw_d(unsigned char *txt, unsigned char *lzw, unsigned int n){
    unsigned short list[4096][64], l = 0, a = 0, c = MC;
    unsigned int i = 0, s = 0;
    bit = 1; n++;
    a = readLZW(lzw, &s);
    while (s < n){
        memset(list, -1, sizeof(list[0][0])*4095*64);
        while ((s < n) && (c != 4096)){
            l = 0;
            if (a < 256)
                list[a][l] = a;
                
            while ((short)list[a][l] != -1){
                txt[i] = list[a][l];
                list[c][l] = list[a][l];
                i++; l++;
            }
            a = readLZW(lzw, &s);
            if (a < 256)
                list[c][l] = a;
            else
                list[c][l] = list[a][0];
            c++;
        }
        c = MC;   
    }
    return i;
}

unsigned int rLe_c(unsigned char *t, unsigned char *z, unsigned int n){
    unsigned int f = 0, i = 0;
    unsigned short c = 0;
    while (i < n){
        c = 1;
        while ((t[i] == t[i + 1]) && (i+1 < n)){
            c++;    i++;
        }
        while (c > 255){
            z[f++] = SP;
            z[f++] = 0xff;
            z[f++] = t[i];
            c = c - 255;
        }
        if ((c < 4) && (t[i] != SP)){
            while (c--)
                z[f++] = t[i];
        }else{
            z[f++] = SP;
            z[f++] = c;
            z[f++] = t[i];
        }
        if ((f - 1) > i++)
            return 0;
    }
    return f;
}

unsigned int rLe_d(unsigned char *z, unsigned char *t, unsigned int n){
    unsigned int i = 0, f = 0;
    unsigned short c = 0;
    while (i < n){
        if (z[i] == SP){    
            c = z[++i];
            ++i;
            while (c--)
                t[f++] = z[i];
        }else
            t[f++] = z[i];
        i++;
    }
    return f;
}

void countBytes(t *t, unsigned char *txt, unsigned int n){
    unsigned int i = -1;
    bit = 0;
    while (n != ++i){
        if(t[bit].p < ++t[txt[i]].p)
            bit = txt[i];
    }
    bit = t[bit].p/255;
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
    if(n < 0)
        n = -n;
    return n;
}

void shannon(unsigned char *s, t *t, unsigned short a, unsigned short b, int n, unsigned short m){
    if ((b - a) != 1){
        int u, p = 0, d1, d2 = MD;
        for (short i = a; i < b; i++){
            u = p;
            p = p + t[s[i]].p;
            d1 = abs(2*p-n);
            d1 = 2*p-n;
            d1 = (d1 < 0) ? -d1 : d1;
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

int payload(t *t, unsigned char *txt, unsigned char *p, unsigned int n){
    unsigned int b = 264, bk = 0, i = -1;
    unsigned short bt = 0, j = 0;
    p[256] = n; p[257] = n>>8; p[258] = n>>16; p[259] = n>>24;

    while (n != ++i){
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
    while (n > i*bit)
        i++;

    if (i > 255)
        i = 255;
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
    printf("Entropia do bloco %.3f bits/simbolo\n", entro);
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
    unsigned char txt[MB], zip[M], s[MC];
    unsigned short o = 0, i = 0;
    unsigned int b = 0, n = 0, lzw = 1, rle = 1;
    while ((n = read(fd, txt, MB))){      
        if (rle){
            if (rle = rLe_c(txt, zip, n)){
                memcpy(txt, zip, rle);
                n = rle;
            }
        }
        
        if (lzw){
            if (lzw = lzw_c(txt, zip, n)){
                memcpy(txt, zip, lzw);
                n = lzw;
            }
        }

        countBytes(t, txt, n);
        
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

        b = payload(t, txt, zip, n);

        if (write(sd, zip, b)<0){
            perror("Erro na escrita do ficheiro compresso");
            return 0;
        }
        memset(&t, 0, sizeof(t));
    }

    return 0;
}

int zipparcomlistagem(short fd, short sd){
    t table[MC] = {0};
    unsigned char txt[MB], zip[M], sorted[MC];
    unsigned short occurrencys = 0, i = 0, numBlock;
    unsigned int blockSize = 0, numChar = MB, original[MC], fileSize = 0, zipSize = 0, lzw = 1, rle = 1;
    float average = 0, blockAverage = 0, entropy = 0;
    char op = 0;
    clock_t t;
    printf("\e[1;1H\e[2J");
    while ((numChar = read(fd, txt, MB))){
        numBlock++;
        fileSize = fileSize + numChar;
        printf("\n---------------------Bloco nr %d---------------------\n\n", numBlock);
        if (rle){
            t = clock();
            if (rle = rLe_c(txt, zip, numChar)){
                printf("Reducao RLE = %.3f%%\n", (float)100 - ((float)rle/numChar)*100);
                printf("Tempo de compressao RLE = %.3fms\n", ((double)(clock()-t)/CLOCKS_PER_SEC)*1000);
                memcpy(txt, zip, rle);
                numChar = rle;
            }else
                printf("Compressao RLE desativada.\n");
        }
        if (lzw){
            t = clock();
            if (lzw = lzw_c(txt, zip, numChar)){
                printf("Reducao LZW = %.3f%%\n", (float)100 - ((float)lzw/numChar)*100);
                printf("Tempo de compressao LZW = %.3fms\n", ((double)(clock()-t)/CLOCKS_PER_SEC)*1000);
                memcpy(txt, zip, lzw);
                numChar = lzw;
            }else
                printf("Compressao LZW desativada.\n");
        }
            
        t = clock();
        countBytes(table, txt, numChar);

        printf("Tempo de leitura e contagem de ocorrencias = %.3fms\n", ((double)(clock()-t)/CLOCKS_PER_SEC)*1000);

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
        printf("Tempo de reducao das ocurrencias = %.3fms\n", ((double)(clock()-t)/CLOCKS_PER_SEC)*1000);

        entropy = entropy + Entropy(original, numChar);
        
        t = clock();
        sort(sorted, table, MC);
        printf("Tempo de ordenacao = %.3fms\n", ((double)(clock()-t)/CLOCKS_PER_SEC)*1000);
        
        t = clock();
        shannon(sorted, table, 0, MC, occurrencys, 0);
        printf("Tempo de Shannon-Fano = %.3fms\n", ((double)(clock()-t)/CLOCKS_PER_SEC)*1000);
        
        blockAverage = compMed(table, original, numChar);
        
        average = average + blockAverage;

        t = clock();
        blockSize = payload(table, txt, zip, numChar);
        if (write(sd, zip, blockSize)<0){
            perror("Erro na escrita do ficheiro compresso");
            return 0;
        }

        printf("Tempo de Construcao e escrita do payload = %.3fms\n", ((double)(clock()-t)/CLOCKS_PER_SEC)*1000);
        
        printf("Comprimento medio do bloco = %.3f \n", blockAverage);

        printf("Mostrar Tabela de Shannon-Fano?\n(s)im ou (n)ao\n");
        op = getchar();
        getchar();
        if (op == 's')
            printTable(table, sorted, original, numChar);

        printf("\n(Pressione enter para continuar)");
        getchar();
        zipSize = zipSize + blockSize;
        memset(&table, 0, sizeof(table));
    }
    printf("\n-------------------------Fim-------------------------\n\nReducao de %.3f%%\n", (float)100 - ((float)zipSize/fileSize)*100 );                   
    printf("Comprimento medio do ficheiro = %.3f\n", average/numBlock);
    printf("Entropia do ficheiro = %.3f bits/simbolo\n", entropy/numBlock);
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
    }else
        r->d = EP;
    return r;
}

void decompress(t *t, struct N *r, unsigned char *c, unsigned char *d, unsigned int n){
    unsigned int ch = 0, b = 0;
    unsigned short bt = 0, i = 0;
    struct N *tp = r;

    while (n != ch){
        for (i = 0; i < 8; ++i){
            bt = (c[b] >> (7-i)) & 1;
            if (bt)    tp = tp->o;
            else    tp = tp->z;
            if (EP ^ tp->d){
                d[ch] = tp->d;
                tp = r;
                if (n == ++ch)    break;
            }
        }
        ++b;
    }
}

int unzippar(short fd, short sd){
    t t[MC] = {0};
    n *r = NULL;

    unsigned char zip[M], txt[MB], s[MC];
    unsigned short o = 0, i = 0;
    unsigned int n = 0, b = 0;
    unsigned char lzw = 1, rle = 1;

    while (read(fd, zip, 264)){
        o = 0; i = 0;
        while (MC != i){
            s[i] = i;
            t[i].p = zip[i];
            o = o + t[i].p;
            ++i;
        }
        n = zip[256]; n += zip[257]<<8; n += zip[258]<<16; n += zip[259]<<24;
        b = zip[260]; b += zip[261]<<8; b += zip[262]<<16; b += zip[263]<<24;

        if (lzw){
            if (n == MB)
                lzw = 0;
        }

        sort( s, t, MC);
        shannon(s, t, 0, MC, o, 0);
        
        i=0;
        while (MC != i){
            r = insert(r, s[i], t[s[i]].c, 0);
            ++i;
        }
        if (read(fd, zip, b)<0){
            perror("Erro na leitura do bloco compresso.");  
            return 0;
        }
        
        decompress(t, r, zip, txt, n);

        if (lzw){
            if (n == MB)
                lzw = 0;
            else
                n = lzw_d(zip, txt, n);
        }
        
        if (rle == 1){
            if (n != MB)
                n = rLe_d(zip, txt, n);
            else
                rle = 0;
        }

        if (write(sd, txt, n)<0){
            perror("Erro na escrita do bloco original.");   
            return 0;
        }

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
