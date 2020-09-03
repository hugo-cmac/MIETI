#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX 30

//nr 9 redes

typedef struct Lista{
    int cod;
    char nome[MAX];
    int quanti;
    float precun;
    struct Lista *next;
}Node, *AptNode;

AptNode Adicionar(AptNode S)
{
	char temp[MAX];
	AptNode nova = NULL;
	nova=(AptNode)malloc(sizeof(Node));

	printf("\nCodigo do arquivo:");
	fgets(temp,MAX,stdin);
	nova->cod=atoi(temp);

	printf("\nNome de artigo:");
	fgets(nova->nome,MAX,stdin);
	nova->nome[strlen(nova->nome)-1]='\0';

	printf("\nQuantidade:");
	fgets(temp,MAX,stdin);
	nova->quanti=atoi(temp);

	printf("\nPreco do artigo:");
	fgets(temp,MAX,stdin);
	nova->precun=atof(temp);

	nova->next = S;

	return nova;
}

void Ler(AptNode S)
{
	while(S!=NULL)
	{
    	printf("\n----Codigo de artigo:%d", S->cod);
    	printf("\n------Nome de artigo:%s", S->nome);
    	printf("\n----------Quantidade:%d", S->quanti);
    	printf("\n------Preco unitario:%.2f\n", S->precun);

    	S = S->next;
    }
}

void Editar(AptNode S)
{
	int opc;
	char temp[MAX];

	AptNode nova = S;

	while(S!=NULL)
	{
    	printf("\n----Codigo de artigo:%d", S->cod);
    	printf("\n------Nome de artigo:%s", S->nome);
    	printf("\n----------Quantidade:%d", S->quanti);
    	printf("\n------Preco unitario:%.2f\n", S->precun);

    	S = S->next;
    }
    printf("\n\nEscreva o codigo do produto que pretende alterar:");
    fgets(temp,MAX,stdin);
	opc=atoi(temp);

	while(nova!=NULL)
	{
		if (opc == nova->cod)
		{
			printf("\nNome de artigo:");
			fgets(nova->nome,MAX,stdin);
			nova->nome[strlen(nova->nome)-1]='\0';

			printf("\nQuantidade:");
			fgets(temp,MAX,stdin);
			nova->quanti=atoi(temp);

			printf("\nPreco do artigo:");
			fgets(temp,MAX,stdin);
			nova->precun=atof(temp);
		}
		nova=nova->next;
	}

}

AptNode Eliminar(AptNode S)
{
	int opc;
	char temp[MAX];


	AptNode nova = S;
	AptNode anterior = S;
	AptNode cabeca = S;


	while(S!=NULL)
	{
    	printf("\n----Codigo de artigo:%d", S->cod);
    	printf("\n------Nome de artigo:%s", S->nome);
    	printf("\n----------Quantidade:%d", S->quanti);
    	printf("\n------Preco unitario:%.2f\n", S->precun);

    	S = S->next;
    }
    printf("\n\nEscreva o codigo do produto que pretende eliminar:");
    fgets(temp,MAX,stdin);
	opc=atoi(temp);

	if(opc == nova->cod)
	{
		cabeca=cabeca->next;
	}
	else
	{
		while(nova!=NULL)
		{
			if ((opc == nova->cod) && (nova->next!=NULL))
			{
				anterior->next = nova->next;
				nova=nova->next;
			}
			else if((opc == nova->cod) && (nova->next==NULL))
            {
                anterior->next=NULL;
                nova=NULL;
            }
            else
            {
                anterior=nova;
                nova=nova->next;
            }
		}
	}

	return cabeca;
}

void gravar(AptNode S)
{
	FILE * Base;

	Base = fopen("armazem.txt","w");

	while(S!=NULL)
	{
		fprintf(Base, "%d\n%s\n%d\n%.2f\n", S->cod, S->nome, S->quanti, S->precun);
    	S = S->next;
    }

    fclose(Base);

}

AptNode lerregistos(AptNode S)
{
	FILE * Base;

	AptNode nova = NULL;
	S = NULL;
	char temp[MAX];


    Base = fopen("armazem.txt", "r");
  		if (Base)
  		{
  			while(!feof(Base))
			{
				nova=(AptNode)malloc(sizeof(Node));

				fgets(temp,MAX,Base);
				nova->cod=atoi(temp);

				fgets(nova->nome,MAX,Base);
				nova->nome[strlen(nova->nome)-1]='\0';

				fgets(temp,MAX,Base);
				nova->quanti=atoi(temp);

				fgets(temp,MAX,Base);
				nova->precun=atof(temp);

				nova->next=S;
				S=nova;
    		}
    		fclose(Base);
		}

		S=S->next;

    return S;
}

int main()
{
	AptNode S = NULL;

	int opc;
	char temp[MAX];

	S = lerregistos(S);

	do{
		printf("\n\n1.Adicionar novo artigo\n2.Ler artigos\n3.Editar artigo\n4.Eliminar artigo\n0.Sair\n");
		fgets(temp,MAX,stdin);
		opc=atoi(temp);

		switch(opc)
		{
			case 1:
			S=Adicionar(S);
			break;

			case 2:
			Ler(S);
			break;

			case 3:
			Editar(S);
			break;

			case 4:
			S=Eliminar(S);
			break;

			case 0:
			break;

			default:
			printf("Opcao invalida.\n");
		}
	}while(opc!=0);

	gravar(S);

	return 0;
}
