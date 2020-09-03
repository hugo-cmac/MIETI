#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MB 2097152//1048576//524288 //512kb
#define MD 99999999
#define MC 256

//gcc -o encoder encoder.c -O3 -march=native
//4.696546 comprimento medio otimo
//4.705440 comprimento medio calculado

typedef struct T{
    unsigned char c[MC];
    unsigned int p;
}t;

unsigned short k = 0;

unsigned int readbytes(short f, t *t, unsigned char *txt){
    unsigned int i = -1, n = read(f, txt, MB);
    k = 0;
    for (size_t f = 0; f < MC; f++){
        printf("%x  %d\n", f, t[f].p);
    }
    getchar();
    while (n != ++i){
        if(t[k].p < ++t[txt[i]].p){
            k = txt[i];
        }
    }
    k = t[k].p/255;

    /*RLE - nOT dONE
    unsigned char z[MB] = {0};
    unsigned int count = 1, bytes = 0;
    for (int i = 0; i<n; i++){
        while (txt[i] == txt[i + 1]){
            count++;
            i++;
        }
        if (count<4){
            ++count;
            while (--count){
                z[bytes] = txt[i];
                ++bytes;
            }
        }else{
            if (count > 255){
                printf("# %x %x\n", count, txt[i]);
            }else{
                z[bytes] = "#";
                z[bytes++] = count;
                z[bytes++] = txt[i];
            }
        }
        count = 1;
    }
    printf("\nOriginal: %d\nRLE: %d\n", n,  bytes+1);
    exit(0);*/
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
        unsigned char f;
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
                    ++i;
                }
                shannon( s, t, a, f, u, m+1);
                shannon( s, t, f, b, n-u, m+1);
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
float calc(t *t, unsigned int *y){
    float m = 0.0;
    unsigned short b = 0, i = -1, f = -1;
    while (MC != ++i){
        if (y[i] != 0){
            b=0;
            f = -1;
            while (MC != ++f){
                if (t[i].c[f] != 0)
                    b++;
                else
                    break;
            }
            m = m + ((float)y[i]/MB)*b;
        }
    }
    return m;
}
void printTable(t *t, unsigned char *s){
    for (int i = 0; i < MC; i++){
        printf("%x -> ", s[i]);
        for (int j = 0; j < MC; j++){
            if (t[s[i]].c[j] != 0){
                printf("%d",t[s[i]].c[j]>>1);
            }else{
                printf("\n");
                break;
            }   
        }
    }
    getchar();
}
int main(int argc, char const *argv[]){
    clock_t tm = clock();
    short fd = 0, sd = 0;
    
    switch (argc){
        case 1:
            perror("Falta indicar o ficheiro para compressao."); return 0;
            break;
        case 2:
            perror("Falta indicar o nome do ficheiro compresso."); return 0;
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
    unsigned short o = 0, i = 0;
    unsigned char txt[MB], zip[MB<<1], s[MC];
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

        write(sd, zip, b);
        memset(&t, 0, sizeof(t));
    }
   
    close(fd);  close(sd);
    printf("Tempo de execucao = %.2fms\n", ((double)(clock()-tm)/CLOCKS_PER_SEC)*1000);
    return 0;
}