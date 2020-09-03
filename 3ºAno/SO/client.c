#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

typedef struct taskStruct{
    int id;
    char date[20];
    char comand[512];
    int exitValue;
    char stdOut[10240];
    char stdErr[1024];
    int estado;
}Task;

struct taskStruct A;

void newTask(int argc, char const *argv[]){
  sprintf(A.stdOut,"<EMPTY>");
  sprintf(A.stdErr,"<EMPTY>");
  sprintf(A.date,"%s %s",argv[2],argv[3]);
  sprintf(A.comand,"%s ",argv[4]);
  for (int i = 5; i < argc; i++) {
    strcat(A.comand,argv[i]);
    strcat(A.comand," ");
  }
  A.id=0;
  A.exitValue=-1;
  A.estado=1;
}
long taskVerif(int argc, char const *argv[]){
  struct tm t;
  int y,m,d,h,min,s;
  long dif;
  sscanf(argv[2],"%d-%d-%d",&y,&m,&d);
  sscanf(argv[3],"%d:%d:%d",&h,&min,&s);
  if (y>2017 && m<13 && d<31 && h<24 && min<60 && s<60) {

    t.tm_year = y-1900;
    t.tm_mon = m-1;
    t.tm_mday = d;
    t.tm_hour = h;
    t.tm_min = min;
    t.tm_sec = s;
    dif=mktime(&t)+3600;
    return dif;
  }else{
    return 0;
  }
}
//./c -a 2018-11-12  23:45:00 /home/fsm/a.out 1 2 3
// 0   1 2           3        4               5 6 7

int main(int argc, char const *argv[]) {
  int fdW;
  int fdR;
  if ((fdW = open("clienttoserv",O_WRONLY))==-1) {
    perror("FifoW error");
    exit(-1);
  }
  if ((fdR = open("servtoclient",O_RDONLY))==-1) {
    perror("FifoR error");
    exit(-1);
  }
  char buffer[1024];

  int n=0,v=0;

  if (argc!=1) {
    //send comand-----------------
    write(fdW,argv[1],strlen(argv[1]));
    //----------------------------
    if (!strcmp("-a",argv[1])) {
        if (argc>=5) {
          if (taskVerif(argc,argv)>time(NULL)) {
            newTask(argc,argv);
            write(fdW,&A,sizeof(Task));
            n=read(fdR,buffer,7);
            write(1,buffer,n);
          }else{
            A.id=-1;
            write(fdW,&A,sizeof(Task));
            sprintf(buffer,"\nComando ou argumento invalido\n");
            write(1,buffer,strlen(buffer));
          }
        }else{
          A.id=-1;
          write(fdW,&A,sizeof(Task));
          sprintf(buffer,"\nComando ou argumento invalido\n");
          write(1,buffer,strlen(buffer));
        }

    }else if (!strcmp("-l",argv[1])) {
      n=1;
      while (n>0) {
        memset(buffer,0,1024);
        n=read(fdR,buffer,1);
        if (buffer[0]=='$') {
          n=0;
        }else{
          write(1,buffer,n);
        }
      }

    }else if (!strcmp("-c",argv[1])) {
      if (argc==3) {
        write(fdW,argv[2],sizeof(argv[2]));
        memset(buffer,0,1024);
        n=read(fdR,buffer,32);
        if (buffer[0]!='$') {
          write(1,buffer,n);
        }
      }else{
        sprintf(buffer,"-1");
        write(fdW,buffer,strlen(buffer));
        n=read(fdR,buffer,32);
        write(1,buffer,n);
      }

    }else if (!strcmp("-r",argv[1])) {
      if (argc==3) {
        write(fdW,argv[2],sizeof(argv[2]));
        read(fdR,&A,sizeof(Task));
        if (A.id!=-1) {
          sprintf(buffer,"ID:\n%d\n\n",A.id);
          write(1,buffer,strlen(buffer));
          sprintf(buffer,"Data:\n%s\n\n",A.date);
          write(1,buffer,strlen(buffer));
          sprintf(buffer,"Linha de comando:\n%s\n\n",A.comand);
          write(1,buffer,strlen(buffer));
          sprintf(buffer,"Valor de saída:\n%d\n\n",A.exitValue);
          write(1,buffer,strlen(buffer));
          sprintf(buffer,"Standart output:\n");
          write(1,buffer,strlen(buffer));
          write(1,A.stdOut,strlen(A.stdOut));
          sprintf(buffer,"\n\nStandart error:\n");
          write(1,buffer,strlen(buffer));
          write(1,A.stdErr,strlen(A.stdErr));
          sprintf(buffer,"\n\n");
          write(1,buffer,strlen(buffer));
        }else{
          sprintf(buffer,"\nComando ou argumento invalido\n");
          write(1,buffer,strlen(buffer));
        }
      }else{
        read(fdR,&A,sizeof(Task));
        sprintf(buffer,"\nComando ou argumento invalido\n");
        write(1,buffer,strlen(buffer));
      }
    }else if (!strcmp("-n",argv[1])) {
      sprintf(buffer,"\nComando indisponivel de momento\n");
      write(1,buffer,strlen(buffer));
    }else if (!strcmp("-e",argv[1])) {
      if (argc==3) {
        sprintf(buffer,"%s",argv[2]);
        write(fdW,buffer,strlen(buffer));
      }else{
        sprintf(buffer,"$");
        write(fdW,buffer,2);
        sprintf(buffer,"\nComando ou argumento invalido\n");
        write(1,buffer,strlen(buffer));
      }
    }else if (!strcmp("help",argv[1])) {
      sprintf(buffer,"\nServiço de execução agendada de programas.\n\n(Para o seu funcionamento correto é necessário executar os servidor, e que os ficheiros client.c e server.c estejam no mesmo local)\n\nAs funcionalidades do serviço são as seguintes:\n\n  -Para agendar tareja: [-a] [Ano-Mês-Dia] [Hora:Minuto:Segundos] [Executável da bash]\n  -Para listar as tarefas: [-l]\n  -Para cancelar uma tarefa: [-c] [id da tarefa]\n  -Para consultar o resultado de uma tarefa: [-r] [id da tarefa]\n  -Para receber o resultado das tarefa no email local: [-e] [mail@localhost]\n\n");
      write(1,buffer,strlen(buffer));
    }else{
      sprintf(buffer,"\nComando ou argumento invalido\n");
      write(1,buffer,strlen(buffer));
    }
  }else{
    sprintf(buffer,"\nComando ou argumento invalido\n");
    write(1,buffer,strlen(buffer));
  }
  close(fdR);
  close(fdW);
  return 0;
}
