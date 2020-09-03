#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <getopt.h>

#define BLOCK_SIZE 512*1024 //512kB

typedef struct {
	uint8_t Symbol; //Símbolo
	uint32_t Freq; //Frequência original de cada símbolo
	uint32_t FreqCopy; //Frequência original de cada símbolo convertida num byte
	char code[300]; //Código binário
	uint8_t Bits; //Tamanho do código
	uint8_t top;
} Symbol;
//Número total de símbolos diferentes de 1 byte
Symbol symbol[256];
//Estrutura da BST
struct BstNode {
    uint8_t Symbol;
    char *code;
    struct BstNode *left;
    struct BstNode *right;
};
struct BstNode *root;
//Número diferente de símbolos presentes no ficheiro
uint16_t nDiffSymbols; 
//Número de bits que ocupa efetivamente o último byte
uint8_t nBits;
uint32_t size;
//---------------------------Instanciação das funções--------------------------------------------------------------------------
void histrogram(uint8_t* str, uint32_t size);
void shannon(int l, int h);
void display();
uint8_t byteToInt(char* s);
void Compress(FILE* file, int verbose);
void writeToFile(uint8_t *str, FILE* zip, uint32_t size);
void oneByteFreq();
void orderByFreqCres();
void stream(uint8_t* str, uint32_t readSize, uint8_t *byteStream);
void makePayload(uint8_t* str, uint32_t size, uint8_t *payload);
void makeHeader(uint8_t *header);
int nBlocks();
uint8_t* intToByte(uint32_t n);
void insert();
char* Search(uint8_t symbolToSearch);
void printUsage();
void printVerbose();

//-------------------------------------------MAIN-----------------------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
	int opt = 0, long_index = 0;
	int verbose = -1, compress = -1;
	char *file = NULL;
	static struct option long_options[] = {
		{"compress", required_argument, 0, 'c'},
		{"verbose",  no_argument,       0, 'v'},
		{0, 0, 0, 0}
	};
	while((opt = getopt_long(argc, argv, "vc:", long_options, &long_index)) != -1) {
		switch(opt) {
			case 'c':
				compress = 0;
				file = optarg;
				break;
			case 'v': verbose = 0;
				break;
			default:
				_exit(1);
		}
	}
	//-------------------------------------------------------------------------------------------
	if(compress == -1 && verbose == -1) {
		printUsage();
		_exit(1);
	}
	FILE* fp = fopen(file, "rb");	
	if(fp == NULL) {
		perror("File could not be open!");
		_exit(1);
	}	
	if(!compress) {
		clock_t start_t = clock();
		Compress(fp, verbose);
		//tempo total
		printf("Compression total time: %.3fms\n", ((double)(clock() - start_t) / CLOCKS_PER_SEC) * 1000);
	}
	fclose(fp);		
	printf("File sucessefully compressed to zip.txt\n");
	return 0;
}

//--------------------------INÍCIO DAS FUNÇÕES------------------------------------------------------------------------
//Função que trata do preenchimento das estruturas
void histrogram(uint8_t* str, uint32_t size) {
	uint32_t i, j;
	uint8_t swaps;
	Symbol temp;
	nDiffSymbols = 0;		
	//Inicializar cada uma das estruturas por ordem de ASCII
	for(i = 0; i < 256; i++) {
		symbol[i].Symbol = i;
		symbol[i].Freq   = 0;
		symbol[i].FreqCopy = 0;
		memset(symbol[i].code, '\0', sizeof(symbol[i].code));
		symbol[i].top    = -1;
	}
	//Número de vezes que aparece cada letra(Frequência)
	for(i = size; i; i--) {
		symbol[*(str)].Freq++;
		symbol[*(str++)].FreqCopy++;
	}
	//Número de símbolos diferentes presentes no ficheiro
	for(i = 0; i < 256; i++) {
		if(symbol[i].Freq != 0) nDiffSymbols++;
	}
	//Ordenar estruturas por Frequência de símbolos
	do {
		swaps = 0;
		for(i = 0; i < 255; i++) {
			if(symbol[i].FreqCopy < symbol[i + 1].FreqCopy) {
				temp = symbol[i];
				symbol[i] = symbol[i + 1];
				symbol[i + 1] = temp;
				swaps = 1;
			}
		}
	}while(swaps); 
	orderByFreqCres();
    oneByteFreq();
    orderByFreqCres();
    //Códigos Shannon-Fano
    if(nDiffSymbols == 1) {
    	symbol[0].code[0] = '0';
    }
    else {
		shannon(0, nDiffSymbols - 1);
    }
	//Tamanho de cada código
	for(i = 0; i < nDiffSymbols; i++) {
		symbol[i].Bits = strlen(symbol[i].code);
	}
}
//Função que serve para fazer o display da Tabela de Shannon-Fano 
void display() {
	printf("\nSymbol\tFrequency\tCode");
	for(int i = nDiffSymbols - 1; i >= 0; i--) {
		printf("\n%d\t%d\t\t%s", symbol[i].Symbol, symbol[i].FreqCopy, symbol[i].code);
	}
	printf("\n");
}
//Função de Compressão do ficheiro original 
void Compress(FILE* file, int verbose) {
	FILE* zip = fopen("zip.txt", "ab");
	if(zip == NULL) {
		perror("File could not be open!\n");
		_exit(1);
	}
	uint8_t chunks[BLOCK_SIZE] = {'\0'};
	if(chunks) {
		while(size = fread(chunks, sizeof(uint8_t), BLOCK_SIZE, file)) {
			//Preenchimento das estruturas
			histrogram(chunks, size);
			//Escever no ficheiro de output
			writeToFile(chunks, zip, size);
			if(!verbose) {
				printVerbose();
			}
			memset(chunks, '\0', size);	
		 }
	}
	fclose(zip);
}
//Cabeçalho 
void makeHeader(uint8_t *header) {
	//Número de blocos de 1byte(8 bits) existente
	uint32_t n = nBlocks();
	//Número diferente de símbolos e respetiva frequência + tamanho da stream de bytes
	uint8_t* payloadSize = intToByte((nDiffSymbols * 2) + n);
	header[0] = nDiffSymbols;
	header[1] = nBits;
	if(payloadSize) {
		memcpy(header + 2, payloadSize, sizeof(uint32_t));
		free(payloadSize);	
	}	
}
//Payload -> Símbolos + frequência dos mesmos + stream de bytes 
void makePayload(uint8_t* str, uint32_t size, uint8_t *payload) {
	//Número de blocos de 1byte(8 bits) existente
	uint32_t n = nBlocks();
	uint8_t byteStream[nBlocks()];
	stream(str, size, byteStream);
	int count = 0;
	for(int i = 0; i < nDiffSymbols; i++) {
		payload[count++] = symbol[i].Symbol;
		payload[count++] = symbol[i].FreqCopy;
	}		
	memcpy(payload + (nDiffSymbols * 2), byteStream, n);
}
//função responsável pela escrita do ficheiro zip
void writeToFile(uint8_t *str, FILE *zip, uint32_t size) {
	Symbol temp;
	//Ordenar estruturas por ASCII 
	for(int i = 0; i < nDiffSymbols; i++) {
		for(int j = i + 1; j < nDiffSymbols; j++) {
			if(symbol[i].Symbol > symbol[j].Symbol) {
				temp = symbol[i];
				symbol[i] = symbol[j];
				symbol[j] = temp;
			}
		}
	}
	//Inserção dos Símbolos e do respetivo código na BST
    insert();
	//Número de blocos de 1byte(8 bits) existente
	uint32_t n = nBlocks();
	uint8_t payload[(nDiffSymbols * 2) + n];
	makePayload(str, size, payload);
	uint8_t  header[6];
	makeHeader(header);

	fwrite(header, sizeof(uint8_t), 6, zip);	
	fwrite(payload, sizeof(uint8_t), (nDiffSymbols * 2) + n, zip);
}
//Função que retorna a stream de bytes 
void stream(uint8_t* str, uint32_t readSize, uint8_t *byteStream) {	
	//Número total de bits(Tamanho de cada código * Frequência do mesmo)
	uint32_t nBinaryCodes = 0; 
	uint32_t i = 0;
	//Ciclo responsável por achar o número total de bits
	for(i = 0; i < nDiffSymbols; i++) {
		nBinaryCodes += symbol[i].Freq * symbol[i].Bits;
	}
	//Array que contém todos os bits(códigos binários)
	char binaryCodes[nBinaryCodes];
	int a = 0;
	//Ciclo que itera todas as letras do ficheiro(uma por uma) 
	for(i = 0; i < readSize; i++) {
		char *code = Search(*(str++));
		while(*code) {
			binaryCodes[a++] = *(code++);
		}	
	}
	/*Caso o valor total de bits seja divisível por 8(1 byte) size fica com o valor do número de bits a dividir por 8,
	 ou seja, size dá o número de blocos de 1 byte cada
	 De notar que o +1 deve-se ao facto do último byte poder não estar totalmente preenchido e o resto da divisão inteira
	 retornar sempre o valor por defeito  
	*/
	int size = nBlocks();
	nBits = CHAR_BIT - ((size * CHAR_BIT) - nBinaryCodes);
	int start = 0, shift = 1;
	//Ciclo que preenche o array temp com o byte correspondente a cada 8 bits
	for(i = 0; i < size; i++) {
		if(i == size - 1) {
			for(int j = start; j < start + nBits; j++) {
				byteStream[i] = byteStream[i] << 1 | binaryCodes[j] - '0';
			}
		}
		else {
			for(int j = start; j < shift<<3; j++) {	
				byteStream[i] = byteStream[i] << 1 | binaryCodes[j] - '0';
			}	
		}
		start = (shift++)<<3;
	}
}
//Função que ordena os Símbolos por Frequência e caso possuam a mesma, por ordem ASCII
void orderByFreqCres() {
	Symbol temp;
	int i, j;
	for (j = 1; j <= nDiffSymbols - 1; j++) { 
		for (i = 0; i < nDiffSymbols - 1; i++) { 
	    	if ((symbol[i].FreqCopy) > (symbol[i + 1].FreqCopy)) { 
	       		temp = symbol[i];
				symbol[i] = symbol[i + 1];
				symbol[i + 1] = temp;
	    	} 
	    	else if((symbol[i].FreqCopy) == (symbol[i + 1].FreqCopy)) {
				if(symbol[i].Symbol < symbol[i + 1].Symbol) {
					temp = symbol[i];
					symbol[i] = symbol[i + 1];
					symbol[i + 1] = temp;
				}
			}
		} 
    } 	
}
//Função que 
void oneByteFreq() {
	while(symbol[nDiffSymbols - 1].FreqCopy > 255) {
		for(int i = 0; i < nDiffSymbols; i++) {
			symbol[i].FreqCopy >>= 1;
		}
	}
}
//Função que retorna o número total de blocos de 1byte 
int nBlocks() {
	int nBinaryCodes = 0;
	for(int i = 0; i < nDiffSymbols; i++) {
		nBinaryCodes += symbol[i].Freq * symbol[i].Bits;
	}
	return (nBinaryCodes % 8) ? ((nBinaryCodes / 8) + 1) : (nBinaryCodes / 8); 
}
//Função Shannon-Fano responsável pela atribuição de um código binário a cada símbolo existente no ficheiro 
void shannon(int l, int h) {  
    int pack1 = 0, pack2 = 0, diff1 = 0, diff2 = 0; 
    int i, d, k, j; 
    if ((l + 1) == h || l == h || l > h) { 
        if (l == h || l > h) {
            return; 
        }
        symbol[h].code[++(symbol[h].top)] = '0'; 
        symbol[l].code[++(symbol[l].top)] = '1'; 
        return; 
    } 
    else { 
        for (i = l; i <= h - 1; i++) {
            pack1 = pack1 + symbol[i].FreqCopy; 
        }
        pack2 = pack2 + symbol[h].FreqCopy; 
        diff1 = pack1 - pack2; 
        if (diff1 < 0) {
            diff1 = diff1 * -1; 
        }
        j = 2; 
        while (j != h - l + 1) { 
            k = h - j; 
            pack1 = pack2 = 0; 
            for (i = l; i <= k; i++) {
                pack1 = pack1 + symbol[i].FreqCopy; 
            }
            for (i = h; i > k; i--) {
                pack2 = pack2 + symbol[i].FreqCopy; 
            }
            diff2 = pack1 - pack2; 
            if (diff2 < 0) {
                diff2 = diff2 * -1; 
            }
            if (diff2 >= diff1) {
                break; 
            }
            diff1 = diff2; 
            j++; 
        } 
        k++; 
        for (i = l; i <= k; i++) {
            symbol[i].code[++(symbol[i].top)] = '1'; 
        }
        for (i = k + 1; i <= h; i++) {
            symbol[i].code[++(symbol[i].top)] = '0'; 
        }
        shannon(l, k); 
        shannon(k + 1, h); 
 	}   
}
//Função que faz a conversão de um inteiro(32 bits) para um conjunto de 4 bytes
uint8_t* intToByte(uint32_t n) {
    uint8_t* bytes = malloc(sizeof(uint8_t) * sizeof(uint32_t)); //4 bytes
    uint8_t aux = 3;
    int size = sizeof(uint32_t);
    for(int i = 0; i < size; i++) {
        bytes[i] = n >> (aux-- << 3);
    }
    return bytes;
}
//Inserir dados na BST sem utilizar recursividade 
void insert() {
    struct BstNode *p, *q;
    for(int i = 0; i < nDiffSymbols; i++) {
        p = (struct BstNode*)malloc(sizeof(struct BstNode));
        p->Symbol = symbol[i].Symbol;
        p->code = symbol[i].code;
        p->left = p->right = NULL;
       
        if(i == 0) {
            root = p;
        }
        else {
            q = root;
            while(1) {
                if(p->Symbol > q->Symbol) {
                    if(q->right == NULL) {
                        q->right = p;
                        break;
                    }
                    else {
                        q = q->right;
                    }
                }
                else {
                    if(q->left == NULL) {
                        q->left = p;
                        break;
                    }
                    else {
                        q = q->left;
                    }
                }
            }
        }
    }   
}
//Função que retorna o respetivo código de cada Símbolo presente no ficheiro 
char* Search(uint8_t symbolToSearch) {
    struct BstNode *q = root;
    int i = 0;
    while(q) {
    	i++;
        if(symbolToSearch > q->Symbol) {
            q = q->right;
        }
        else if(symbolToSearch < q->Symbol) {
            q = q->left;
        }
        else {
            return (q->code);
        }
    }
}   
//Modo de uso 
void printUsage() {
    printf("Usage: Compress [v] -c input_file_name.extension\n");
}

void printVerbose() {
	orderByFreqCres();
	int i = 0;
	//Tamanho de cada bloco de leitura
	printf("Block Size: %dkB\n", BLOCK_SIZE);
	//Valor da Entropia(Informação média(por símbolo) gerada pela fonte)
	double entropy = 0, cCode = 0;
	for(i = 0; i < nDiffSymbols; i++) {
		double prob = (double)symbol[i].Freq / size;
		entropy += prob * log2(1/prob); //Entropia
		cCode   += prob * symbol[i].Bits; //Comprimento médio do código 
	}
	printf("Entropia: %.3f bits/símbolo\n", entropy);
	//Tabela Shannon-Fano
	//display();
	printf("Comprimento médio do código: %.3f dig bin/símbolo\n", cCode);
	double compression = ((8 - cCode) / 8) * 100;
	if(compression < 0) compression = -compression;
	printf("Compressão de cada bloco: %.3f%c\n", compression, '%');
}

