#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX1 100
#define MAX2 10

typedef struct A //Estrutura do aluno
{
  int nrdealuno;
  char nome[MAX1];
  char curso[MAX2];
  int ano;
  int avaliacao;
} tAluno;

tAluno A1[MAX1];

void novoregisto()
{
    //leitura dos dados do novo registo
    tAluno aluno;
    printf("Digite o nr de aluno: ");
    scanf("%d", &aluno.nrdealuno);

    printf("Digite o nome do aluno: ");
    scanf("%s", aluno.nome);

    printf("Digite o curso do aluno: ");
    scanf("%s", aluno.curso);

    printf("Digite o ano em que o aluno se encontra escrito: ");
    scanf("%d", &aluno.ano);

    printf("Digite a avaliacao do aluno: ");
    scanf("%d", &aluno.avaliacao);
    printf("\n");

    //inicializaçao do ficheiro dentro da funçao novoregisto
    FILE *apFile;

    if(apFile==(FILE*) NULL)		//verificao da existencia do ficheiro
    {
        apFile = fopen("dados.txt", "w");
        fwrite( &aluno, sizeof(aluno), 1, apFile );
        fclose(apFile);
    }
    else
    {
        apFile = fopen("dados.txt", "a");
        fwrite( &aluno, sizeof(aluno), 1, apFile );
        fclose(apFile);
    }
}

void lerregistos()		//funcao lerregistos
{
    int i=0;
    int j;
    FILE * apFile;
    apFile = fopen("dados.txt", "r");	//chamar o ficheiro para leitura
    tAluno aluno;

    if(apFile!=(FILE*) NULL)
    {
        while(!feof(apFile))
        {
        fread(&A1[i],sizeof(aluno),1,apFile);		//escrever o conteudo do ficheiro no array de estrutura
        i++;
        }
    }
    fclose(apFile);
    for( j=0; j<i-1; j++)
    {
    printf("\n\n\n%d", j);
    printf("\n   Numero do aluno: %d", A1[j].nrdealuno);
    printf("\n     Nome de aluno: %s", A1[j].nome);
    printf("\n             Curso: %s", A1[j].curso);
    printf("\n      Ano do aluno: %d", A1[j].ano);
    printf("\nAvaliacao do aluno: %d\n", A1[j].avaliacao);
    }
}

void atualizarRegisto()		//inicializacao da funcao actualizar registos
{
    int escolha;
    int i=0;
    int j;
    FILE * apFile;
    apFile = fopen("dados.txt", "r");
    tAluno aluno;

    if(apFile!=(FILE*) NULL)
    {
        while(!feof(apFile))
        {
        fread(&A1[i],sizeof(aluno),1,apFile);
        i++;
        }
    }
    fclose(apFile);

    printf("\n   Numero do aluno a atualizar:  ");
    scanf("%d",&escolha);

    for( j=0; j<i-1; j++)
    {
        if(escolha==A1[j].nrdealuno)				//comparacao de nr de aluno para fazer a selecao do respetivo registo para atualizaçao
        {
        	printf("Digite o nr de aluno: ");
    		scanf("%d", &aluno.nrdealuno);

            printf("Digite o nome do aluno: ");
            scanf("%s", A1[i].nome);

            printf("Digite o curso do aluno: ");
            scanf("%s", A1[j].curso);

            printf("Digite o ano em que o aluno se encontra escrito: ");
            scanf("%d", &A1[j].ano);

            printf("Digite a avaliacao do aluno: ");
            scanf("%d", &A1[j].avaliacao);
        }
    }

    apFile = fopen("dados.txt", "w");			//gravar os registos num ficheiro limpo
    for( j=0; j<i-1; j++)
    {
        fwrite( &A1[j], sizeof(A1[j]), 1, apFile );
    }
    fclose(apFile);
}

void eliminarRegisto()
{
    int escolha;
    int i=0;
    int j;
    FILE * apFile;
    apFile = fopen("dados.txt", "r");
    tAluno aluno;

    if(apFile!=(FILE*) NULL)
    {
        while(!feof(apFile))
        {
        fread(&A1[i],sizeof(aluno),1,apFile);
        i++;
        }
    }
    fclose(apFile);

    printf("\n   Número do aluno a eliminar:  ");
    scanf("%d",&escolha);

    for( j=0; j<i-1; j++)
    {
        if(escolha==A1[j].nrdealuno)
            A1[j].nrdealuno=-1;				//igualamos a variavel nrdealuno a menos
    }

    apFile = fopen("dados.txt", "w");
    for( j=0; j<i-1; j++)
    {
        if(A1[j].nrdealuno!=-1)				//para agora gravar no ficheiro todos os registo exceto o o array que contem -1
            fwrite( &A1[j], sizeof(A1[j]), 1, apFile );
    }
    fclose(apFile);
}

int main()
{
  int opcaomenu;

do
{

  printf("\r1.Criar novo registo\n\r2.Ler registos\n3.Atualizar registo\n4.Eliminar registo.\n0.Sair \n");

  scanf("%d", &opcaomenu);

  switch(opcaomenu)
  {
    case 1:
        novoregisto();
        break;

    case 2:
        lerregistos();
        break;

    case 3:
        printf("Atualizar registo\n");
        atualizarRegisto();
        break;

    case 4:
        printf("Eliminar registo\n");
        eliminarRegisto();
        break;
    case 0:
        break;

    default :
        printf("Opcao invalida\n");
  }
}while(opcaomenu!=0);
  return 0;
}