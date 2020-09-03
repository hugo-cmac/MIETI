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
    int estado;//1-activo/2-Executado/0-Cancelado
}Task;

struct taskStruct A[256];
struct taskStruct T;
int idx,status,size,out[2],error[2];
int fdR,fdW;

void printTasks(int fdW){
  int V=0,s=0;
  char buffer[1024];

  if (size!=0) {
    for (int i = 0; i < size; i++) {
      if (A[i].estado==1) {
        if (s==0) {
          sprintf(buffer,"Agendadas:\n");
          write(fdW,buffer,strlen(buffer));
          s=1;
        }
        sprintf(buffer,"%d %s %s\n", A[i].id, A[i].date, A[i].comand);
        write(fdW,buffer,strlen(buffer));
      }else if(A[i].estado==2){
        V=1;
      }}

    if (V==1) {
      sprintf(buffer,"Executadas:\n");
      write(fdW,buffer,strlen(buffer));
      for (int j = 0; j < size; j++){
        if (A[j].estado==2) {
          sprintf(buffer,"%d %s %s\n", A[j].id, A[j].date, A[j].comand);
          write(fdW,buffer,strlen(buffer));
        }}}
  }
  buffer[0]='$';
  write(fdW,buffer,1);
}
int nextTask(){
  long t1,t2=1000000000000000000;
  int y,m,d,h,min,s,index=-1;
  struct tm task;
  for (int i = 0; i < size; i++) {
    if (A[i].estado==1) {
      sscanf(A[i].date,"%d-%d-%d %d:%d:%d",&y,&m,&d,&h,&min,&s);
      task.tm_year = y-1900;
      task.tm_mon = m-1;
      task.tm_mday = d;
      task.tm_hour = h;
      task.tm_min = min;
      task.tm_sec = s;
      t1 = mktime(&task);

      if (t2>t1) {
        index=i;
        t2=t1;
        printf("%ld\n",t2 );
      }
    }
  }
  if (index==-1) {
    index=idx;
  }
  return index;
}
char **toComand(){
    char **c;
    int w=0,i;
    for (i = 0; i < strlen(T.comand); i++) {
      if(T.comand[i]==' '){
        w++;
      }
    }
    i=0;
    c = malloc((w+1)*sizeof(char*));
    c[i] = strtok(T.comand," ");
    i++;
	  while (c[i] != NULL){
		     c[i] = strtok(NULL," ");
         i++;
	   }
     if ((w=fork())==-1) {
       perror("Fork error");
       exit(-1);
     }
    if(w==0){
      free(c);
      _exit(0);
    }else{
      wait(0);
      return c;
    }
}
long taskOn(){
  struct tm task;
  int y,m,d,h,min,s;
  long t,n,a;
  sscanf(T.date,"%d-%d-%d %d:%d:%d",&y,&m,&d,&h,&min,&s);
  task.tm_year = y-1900;
  task.tm_mon = m-1;
  task.tm_mday = d;
  task.tm_hour = h;
  task.tm_min = min;
  task.tm_sec = s;
  t = mktime(&task);
  n = time(NULL);
  a=(t-n);
  printf("%ld\n",a);
  if (a==0 || a<0) {
    a=1;
  }
  return a;
}
void chooseTask(){
  int temp;
  long tsec;
  temp=nextTask(size);
  if (temp!=idx) {
    idx=temp;
    T=A[idx];
    tsec=taskOn();
    printf("%ld\n",tsec);
    alarm(tsec);
    }
  }
void sigchld_handler(int sigC){
    wait(&status);
    A[idx].exitValue=WEXITSTATUS(status);
    A[idx].estado=2;

    close(out[1]);close(error[1]);
    read(out[0],A[idx].stdOut, 10240);
    read(error[0],A[idx].stdErr, 1024);
    close(out[0]);close(error[0]);
    chooseTask();
}
void sigalrm_handler(int sigA){
    char **cmd;
    int son;
    signal(SIGCHLD, NULL);
    cmd=toComand();
    if ((pipe(out))==-1) {
      perror("Pipe error");
      exit(-1);
    }
    if ((pipe(error))==-1) {
      perror("Pipe error");
      exit(-1);
    }
    if ((son=fork())==-1) {
      perror("Fork error");
      exit(-1);
    }
    if (son==0) {
      dup2(out[1],1);
      dup2(error[1],2);
      close(out[0]);close(out[1]);
      close(error[0]);close(error[1]);
        if (execvp(cmd[0],cmd) == -1){
          perror("Execvp error");
    		  exit(-1);
        }
    }else{
      free(cmd);
      signal(SIGCHLD, sigchld_handler);
    }
}
void sigint_handler(int sigI) {
  int n=0;
  char buffer[1024];
  sprintf(buffer,"\nTem a certeza que quer desligar o servidor?\n(s/n)\n");
  write(1,buffer,strlen(buffer));
  while (n!=1) {
    memset(buffer,0,1024);
    n=read(0,buffer,1024);
    if (buffer[0]=='n' || buffer[0]=='s') {
      n=1;
    }else{
      n=0;
    }
  }
  if (buffer[0]=='s') {
    close(fdR);
    close(fdW);
    unlink("clienttoserv");
    unlink("servtoclient");
    exit(0);
  }else{
    signal(SIGINT, sigint_handler);
  }
}

int main(int argc, char const *argv[]) {
  mkfifo("clienttoserv",0666);
  mkfifo("servtoclient",0666);

  if ((fdR = open("clienttoserv",O_RDONLY))==-1) {
    perror("FifoR error");
    exit(-1);
  }
  if ((fdW = open("servtoclient",O_WRONLY))==-1) {
    perror("FifoW error");
    exit(-1);
  }
  signal(SIGALRM, sigalrm_handler);
  signal(SIGINT, sigint_handler);

  struct taskStruct B;

  char buffer[1024],cmd[3];
  int nC,n,temp,tsec;
  idx=-1;
  size=0;
  while (1) {
    //pass comand-------------
    n=0;
    while (n==0) {
      n=read(fdR,cmd,2);
      cmd[2]='\0';
    }
    //---------------------------
    if (!strcmp("-a",cmd)) {
      read(fdR,&B,sizeof(Task));
      if (B.id!=-1) {
        size++;
        A[size-1]=B;
        A[size-1].id=size;
        sprintf(buffer,"%d\n",A[size-1].id);
        write(fdW,buffer,strlen(buffer));
        chooseTask();
      }
    }else if (!strcmp("-l",cmd)) {
      printTasks(fdW);
    }else if (!strcmp("-c",cmd)) {
      n=read(fdR,buffer,5);
      sscanf(buffer,"%d",&nC);
      if (nC<=size && nC!=-1) {
        if ((nC-1)==idx) {
          A[idx].estado=0;
          temp=nextTask();
          if (temp!=idx) {
            idx=temp;
            T=A[idx];
            tsec=taskOn();
            alarm(tsec);
          }else if (temp==idx){
            alarm(0);
          }
          buffer[0]='$';
          write(fdW,buffer,1);
        }else{
          for (int i = 0; i < size; i++) {
            if (nC==(i+1)) {
              A[i].estado=0;
            }
          }
          buffer[0]='$';
          write(fdW,buffer,1);
        }
      }else{
        sprintf(buffer,"\nComando ou argumento invalido\n");
        write(fdW,buffer,strlen(buffer));
      }
    }else if (!strcmp("-r",cmd)) {
      n=read(fdR,buffer,5);
      sscanf(buffer,"%d",&nC);
      if ((nC<=size || A[nC-1].estado==2) && nC!=-1) {
        write(fdW,&A[nC-1],sizeof(Task));
      }else{
        B.id=-1;
        write(fdW,&B,sizeof(Task));
      }
    }else if (!strcmp("-n",cmd)) {
    }else if (!strcmp("-e",cmd)) {
      char mailAd[128];
      char mailbody[10240];
      int mail[2],son;
      n=read(fdR,mailAd,128);
      mailAd[strlen(mailAd)]='\0';

      if (mailAd[0]!='$') {
        for (int i = 0; i < size; i++) {
          if (A[i].estado==2) {
            if (pipe(mail)==-1) {
              perror("pipe error");
              exit(-1);
            }
            signal(SIGCHLD, NULL);
            if ((son=fork())==-1) {
              perror("Fork error");
              exit(-1);
            }
            if(son==0){
              dup2(mail[0],0);
              close(mail[0]);close(mail[1]);
              if ((execlp("mail","mail","-s",A[i].comand,mailAd,NULL))==-1) {
                perror("Mail error");
                exit(-1);
                }
            }else{
              sprintf(mailbody,"ID:\n%d\n\n",A[i].id);
              sprintf(buffer,"Data:\n%s\n\n",A[i].date);
              strcat(mailbody,buffer);
              sprintf(buffer,"Linha de comando:\n%s\n\n",A[i].comand);
              strcat(mailbody,buffer);
              sprintf(buffer,"Valor de saída:\n%d\n\n",A[i].exitValue);
              strcat(mailbody,buffer);
              sprintf(buffer,"Standart output:\n");
              strcat(mailbody,buffer);
              strcat(mailbody,A[i].stdOut);
              sprintf(buffer,"\n\nStandart error:\n");
              strcat(mailbody,buffer);
              strcat(mailbody,A[i].stdErr);
              sprintf(buffer,"\n\n");
              strcat(mailbody,buffer);
              write(mail[1],mailbody,strlen(mailbody));
              close(mail[0]);close(mail[1]);
              wait(&status);
            }
            }
          }
      }
    }
  }
  return 0;
}
