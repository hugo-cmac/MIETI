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
#include <math.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <mysql/my_global.h>
#include <mysql/mysql.h>

#define BUFFERSIZE 420
#define NCLIENTS 10

typedef struct Area{
  int pid;
  int pipe[2];
  int isu;
} AreaGestor;

typedef struct Client {
  int pid;
  int socket;
} ClientGest;
struct Client client[NCLIENTS];


unsigned char buffer[BUFFERSIZE];
int gestor=1;
int setupFile;
int socketfd;
int pidTemp;
float queda=0;
int tipocomp;
//gcc -Wall gestor.c -lm `mysql_config --cflags --libs`

ssize_t readln(int fildes, void *buf, ssize_t nbyte){
	int i = 0;
	while((read(fildes, buf+i, nbyte )) != 0) {
		if(*(char*)(buf+i) == '\n'){
      i++;
      break;
		}
		i++;
	}
	return i;
}

int finish_with_error(MYSQL *con){
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  return 1;
}

int createDatabase(){
  MYSQL *con = mysql_init(NULL);

  if (con == NULL){
    finish_with_error(con);
  }
  if (mysql_real_connect(con, "localhost", "lti2", "lti2", NULL, 0, NULL, 0) == NULL){
    return finish_with_error(con);
  }
  if (mysql_query(con, "CREATE DATABASE IF NOT EXISTS monitorizacao_fisica")) {
    return finish_with_error(con);
  }
  mysql_close(con);
  return 0;
}

int createTable(int area){
  MYSQL *con = mysql_init(NULL);
  char query[256];

  if (con == NULL){
      return finish_with_error(con);
  }
  if (mysql_real_connect(con, "localhost", "lti2", "lti2", "monitorizacao_fisica", 0, NULL, 0) == NULL){
      return finish_with_error(con);
  }
  sprintf(query,"CREATE TABLE IF NOT EXISTS Area_%d_Values( ISS INT, ISu INT, Data DATE, Hora TIME, acX FLOAT, acY FLOAT, acZ FLOAT, gX FLOAT, gY FLOAT, gZ FLOAT, Temp FLOAT)",area);
  if (mysql_query(con, query)) {
      finish_with_error(con);
  }
  sprintf(query,"CREATE TABLE IF NOT EXISTS Area_%d_Behaviour( ISS INT, ISu INT, Data DATE, Tempo TIME, Comportamento TEXT)",area);
  if (mysql_query(con, query)) {
      finish_with_error(con);
  }
  mysql_close(con);
  return 0;
}

int bit32Read(int i){
  int a;
  a = (int)buffer[i]*pow(2,24)+(int)buffer[i+1]*pow(2,16)+(int)buffer[i+2]*pow(2,8)+(int)buffer[i+3]*1;
  return a;
}

float adjustNegative(int n){
  int tmp = bit32Read(n);
  if (tmp>510818) {
    tmp=tmp-1200000;
  }
  return ((float)tmp/1000.00);
}

void timeStruct(time_t ts,char timeStamp[30]){
  struct tm * t;
  t = localtime((const time_t *) &ts);
  sprintf(timeStamp,"'%d-%d-%d', '%d:%d:%d'", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
}

void timePost(time_t ts,char timepost[64]){
  struct tm * t;
  t = localtime((const time_t *) &ts);
  sprintf(timepost,"%d-%d-%d&tempo=%d:%d:%d", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
}

float vector(float sx,float sy,float sz){
  sx=sx*sx;
  sy=sy*sy;
  sz=sz*sz;
  return sqrt(sx+sy+sz);
}

void behaviourCalc(float diff,float s, int n,float ax,float ay,float az,char behaviour[64]) {
  diff=s/n;
  if (diff==0.0) {
    if (fabs(ax)==0.0 || fabs(ay)==0.0 || fabs(az)==0.0) {
      sprintf(behaviour,"'Alarme - Inativo'");
      tipocomp=0;
    }
  }else if (diff<0.07) {
    sprintf(behaviour,"'Normal -");
    if (fabs(ax)>0.90 || fabs(az)>0.90) {
      if (queda>0.20) {
        sprintf(behaviour,"'Alarme - Queda'");
        tipocomp=6;
      }else{
        strcat(behaviour," Deitado'");
        tipocomp=1;
      }
    }else if (fabs(ay)>0.90){
      strcat(behaviour," Levantado'");
      tipocomp=2;
    }else{
      strcat(behaviour," Má postura'");
      tipocomp=3;
    }
  }else if(diff>0.07 && diff<0.22){
    sprintf(behaviour,"'Normal - Movimento(andar)'");
    tipocomp=4;
  }else if(diff>0.22 && diff<0.50){
    sprintf(behaviour,"'Alarme - Movimento(correr)'");
    tipocomp=5;
  }
  queda=diff;
}

void cutString(char timeStamp[30],time_t ts){
  struct tm * t;
  t = localtime((const time_t *) &ts);
  sprintf(timeStamp,"%d:%d:%d", t->tm_hour, t->tm_min, t->tm_sec);
}

void makeFifo(int isu,char fifoName[20]){
  sprintf(fifoName,"Real/ISu_%d",isu);
  mkfifo(fifoName,0666);
}

void execPost(char p[256]){
  if (!fork()) {
    close(2);close(1);close(0);
    system(p);
    exit(0);
  }
}

void sigchld_post(int sig) {
  int s;
  wait(&s);
}

void printBuffer() {
  for (int i = 0; i < 50; i++) {
    printf("%u",buffer[i]);
  }
  printf("\n");
}

void insertValues( int pipe, int area, int isu){
  signal(SIGCHLD, sigchld_post);
  MYSQL *con = mysql_init(NULL);
  char query[256],timeStamp[30],behaviour[64],fifoName[20],post[256],timepost[64];
  int n,iss,ns,ts,samplesRecv=-1,fifo=-1,f=-1;
  float acX,acY,acZ,gX,gY,gZ,temp;
  float nowVector,previousVector,difference=0.0,sum=0.0;
  if (con == NULL){ finish_with_error(con);  }
  if (mysql_real_connect(con, "localhost", "lti2", "lti2", "monitorizacao_fisica", 0, NULL, 0) == NULL){  finish_with_error(con);  }
  makeFifo(isu,fifoName);
  while ((n=read(pipe,buffer,sizeof(buffer)))>0) {
    iss = bit32Read(4);
    ns = bit32Read(12);
    ts = bit32Read(16);
    timeStruct(ts, timeStamp);
    timePost(ts, timepost);
    n=16;
    for (int i = 0; i < ns; i++) {
      samplesRecv++;
      n=n+4;    acX=adjustNegative(n);
      n=n+4;    acY=adjustNegative(n);
      n=n+4;    acZ=adjustNegative(n);
      n=n+4;    gX=adjustNegative(n);
      n=n+4;    gY=adjustNegative(n);
      n=n+4;    gZ=adjustNegative(n);
      n=n+4;    temp=adjustNegative(n);

      memset(query,'\0',BUFFERSIZE);memset(post,'\0',BUFFERSIZE);
      sprintf(query,"INSERT INTO Area_%d_Values VALUES( %d, %d, %s, %f, %f, %f, %f, %f, %f, %f)",area,iss,isu,timeStamp,acX,acY,acZ,gX,gY,gZ,temp);
      sprintf(post,"curl \"http://lti2moni.000webhostapp.com/in_valor.php?id=%d&iss=%d&gest=%d&area=%d&data=%s&ax=%.2f&ay=%.2f&az=%.2f&gx=%.2f&gy=%.2f&gz=%.2f&tmp=%.2f\"",isu,iss,gestor,area,timepost,acX,acY,acZ,gX,gY,gZ,temp);
      execPost(post);
      if (mysql_query(con, query)) {  finish_with_error(con);  }
      nowVector=vector(acX,acY,acZ);
      if (samplesRecv){
        difference=nowVector-previousVector;
        if (difference<0) {   difference=-difference;}
      }
      previousVector=nowVector;
      sum=sum+difference;
    }
    if (samplesRecv==ns) {
      behaviourCalc(difference,sum,(ns-1),acX,acY,acZ,behaviour);  //verificar numero de amostras
    }else{
      behaviourCalc(difference,sum,ns,acX,acY,acZ,behaviour);
    }
    sum=0;
    memset(query,'\0',BUFFERSIZE);memset(post,'\0',BUFFERSIZE);

    sprintf(query,"INSERT INTO Area_%d_Behaviour VALUES( %d, %d, %s, %s)",area,iss,isu,timeStamp,behaviour);
    sprintf(post,"curl \"http://lti2moni.000webhostapp.com/in_comp.php?id=%d&iss=%d&gest=%d&area=%d&data=%s&comp=%d\"",isu,iss,gestor,area,timepost,tipocomp);
    execPost(post);
    if (mysql_query(con, query)) {  finish_with_error(con);  }
    cutString(timeStamp,ts);
    memset(query,'\0',BUFFERSIZE);
    sprintf(query,"Area - %d\tISU - %d\tTempo - %s   \tComportamento - %s\n",area,isu,timeStamp,behaviour);
    if (fifo<0 || f<0) {
      fifo=open(fifoName, O_WRONLY | O_NONBLOCK);
    }
    if (!(fifo<0)) {
      f=write(fifo,query,strlen(query));
    }
  }
  memset(query,'\0',BUFFERSIZE);
  if (!(fifo<0)) {
    close(fifo);
  }
  sprintf(query,"Real/ISu_%d",isu);
  unlink(query);
  close(pipe);
  mysql_close(con);
  exit(0);
}

void sigint_recvMsg(int sig) {
  for (int i = 0; i < NCLIENTS; i++) {
    if (client[i].socket!=0) {
      close(client[i].socket);
    }
  }
}

void sigchld_recvMsg(int sig){
  int status;
  pidTemp=wait(&status);
}

void recvMsg(int area, int this){
  signal(SIGINT,sigint_recvMsg);
  signal(SIGCHLD, sigchld_recvMsg);
  struct Area AreaX[NCLIENTS];
  int n=-1,tp,c=1;

  for (int i = 0; i < NCLIENTS; i++) {  memset(&AreaX[i],0,sizeof(AreaX[i]));  }
  while ((c=recv(client[this].socket,buffer,sizeof(buffer),0))>0) {
    tp = bit32Read(0);
    if (tp==0) {
      /* Start */
      n++;
      AreaX[n].isu = bit32Read(8);
      pipe(AreaX[n].pipe);
      if ((AreaX[n].pid=fork())<0) {
        perror("fork error");
        exit(-1);
      }else if (AreaX[n].pid==0){
        close(AreaX[n].pipe[1]);
        close(client[this].socket);
        insertValues( AreaX[n].pipe[0], area, AreaX[n].isu);
      }
      close(AreaX[n].pipe[0]);
    }else if (tp==2 || tp==3){
      int s = bit32Read(8);
      for (int i = 0; i < NCLIENTS; i++) {
        if (AreaX[i].isu==s) {
          close(AreaX[i].pipe[1]);
          AreaX[i].pipe[1]=0;
          if (tp==2) {
            printf("Monitorização do iSu: %d, terminou normalmente\n", AreaX[i].isu);
          }else{
            printf("Monitorização do iSu: %d, terminou devido a um erro\n", AreaX[i].isu);
          }
        }
      }
    }else if (tp==1) {
      int s = bit32Read(8);
      for (int i = 0; i < NCLIENTS; i++) {
        if (AreaX[i].isu==s) {
          write(AreaX[i].pipe[1],buffer,sizeof(buffer));
        }
      }
    }
    if (pidTemp!=0) {
      for (int i = 0; i < NCLIENTS; i++) {
        if (AreaX[i].pid==pidTemp) {
          pidTemp=0;
          AreaX[i].pid=0;
          close(AreaX[i].pipe[0]);
        }
      }
    }
    memset(buffer,'\0',BUFFERSIZE);
  }
  for (int i = 0; i < NCLIENTS; i++) {
    if (AreaX[i].pipe[1]!=0) {
      close(AreaX[i].pipe[1]);
    }
  }
  if (c==0) {
    printf("Area %d foi desconnectada.\n",area);
  }
  exit(0);
}

void recvAreaId(int this){
  int tp=1;
  int area;
  while (tp!=4) {
    recv(client[this].socket,(char *)buffer,sizeof(buffer),0);
    tp=bit32Read(0);
  }
  area=bit32Read(4);
  if (createTable(area)!=0) {
    perror("Error creating DB table.\n");
    exit(-1);
  }
  printf("Connectado a Area %d.\n(pressione enter)\n",area );
  recvMsg(area,this);
}

void newArea(){
  int i=0;
  printf("Ligue o concentrador para estabelecer a ligaçao\n");
  memset(buffer,'\0',BUFFERSIZE);
  if (listen( socketfd, 5)<0) {
    perror("Listen error.\n");
    sprintf((char *)buffer,"Listen error.\n");
    write(setupFile,(char *)buffer,strlen((char *)buffer));
    exit(-1);
  }else{
    sprintf((char *)buffer,"Listen successful.\nWaiting new connection.\n");
    write(setupFile,(char *)buffer,strlen((char *)buffer));
  }
  i++;
  client[i].socket = accept(socketfd, NULL, NULL);
  if (client[i].socket < 0) {
    perror("Client Socket error.\n");
    sprintf((char *)buffer,"Client %d Socket error.\n",i);
    write(setupFile,(char *)buffer,strlen((char *)buffer));
    exit(-1);
  }else{
    sprintf((char *)buffer,"Client %d Socket creation successful.\n",i);
    write(setupFile,(char *)buffer,strlen((char *)buffer));
    client[i].pid=fork();
    if (client[i].pid < 0) {
      perror("fork error.\n");
      exit(-1);
    }else if (client[i].pid == 0) {
      close(setupFile);
      close(socketfd);
      recvAreaId(i);
    }
  }
}

void configuration(){
  int p,a,n,m,portTCP,configfd;
  char ip[10];
  for (int i = 0; i < 10; i++) {
    memset(&client[i],0,sizeof(client[i]));
  }
  memset(buffer,'\0',BUFFERSIZE);
  if ((setupFile = open("setupTCPserver.txt",O_CREAT | O_WRONLY | O_TRUNC ,0666))<0) {
    perror("setup log file error");
    sprintf((char *)buffer,"Setup log file error.\n");
    write(setupFile,(char *)buffer,strlen((char *)buffer));
    exit(-1);
  }else{
    sprintf((char *)buffer,"Setup log file created.\n");
    write(setupFile,(char *)buffer,strlen((char *)buffer));
  }
  memset(buffer,'\0',BUFFERSIZE);
  if ((configfd=open("config.txt",O_RDONLY))<0) {
    perror("Error reading config file\n");
    sprintf((char *)buffer,"Error reading config file\n");
    write(setupFile,(char *)buffer,strlen((char *)buffer));
    exit(-1);
  }else{
    read(configfd,(char *)buffer,BUFFERSIZE);
    sscanf((char *)buffer,"%d\n%d\n%d\n%d\n%d\n%s",&p,&a,&n,&m,&portTCP,ip);
    close(configfd);
    sprintf((char *)buffer,"Config file read.\n");
    write(setupFile,(char *)buffer,strlen((char *)buffer));
  }
  memset(buffer,'\0',BUFFERSIZE);
  if (createDatabase()<0) {
    perror("DataBase creation error.\n");
    sprintf((char *)buffer,"DataBase creation error.\n");
    write(setupFile,(char *)buffer,strlen((char *)buffer));
    exit(-1);
  }else{
    sprintf((char *)buffer,"DataBase creation successful.\n");
    write(setupFile,(char *)buffer,strlen((char *)buffer));
  }
  memset(buffer,'\0',BUFFERSIZE);
  if ( (socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		perror("TCP socket error");
    sprintf((char *)buffer,"TCP socket error.\n");
    write(setupFile,(char *)buffer,strlen((char *)buffer));
    exit(-1);
	}else{
    sprintf((char *)buffer,"TCP socket file descriptor created.\n");
    write(setupFile,(char *)buffer,strlen((char *)buffer));
  }
  struct    sockaddr_in     server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(portTCP);
  server_address.sin_addr.s_addr = INADDR_ANY;
  memset(buffer,'\0',BUFFERSIZE);
  if ((bind( socketfd, (struct sockaddr *) &server_address, sizeof(server_address)))<0) {
    perror("Socket binded error.\n");
    sprintf((char *)buffer,"Socket binded error.\n");
    write(setupFile,(char *)buffer,strlen((char *)buffer));
    exit(-1);
  }else{
    sprintf((char *)buffer,"Socket bind successful.\n");
    write(setupFile,(char *)buffer,strlen((char *)buffer));
  }
}

void sigchld_main(int sig) {
  int status,p;
  p=wait(&status);
  for (int i = 0; i < NCLIENTS; i++) {
    if (client[i].pid==p) {
      client[i].pid=0;
    }
  }
}

void realTimeM() {
  signal(SIGCHLD, NULL);
  int isu;
  printf("ISu's atualmente a serem monitorizados\n\n");
  if (!fork()) {
    int p[2];
    pipe(p);
    if (!fork()) {
      close(p[0]);
      dup2(p[1],1);
      execlp("ls","ls","Real/",NULL);
    }else{
      close(p[1]);
      dup2(p[0],0);
      execlp("cut","cut","-f1",NULL);
    }
  }
  wait(NULL);
  printf("\n\nIntroduza o nr do ISu\n(0-Para voltar para o menu principal)\n");
  scanf("%d",&isu);
  if (isu!=0) {
    if (!fork()) {
      char exec[20];
      sprintf(exec,"./monitor %d",isu);
      execlp("gnome-terminal","gnome-terminal","--","bash","-c",exec,NULL);
    }
  }
  signal(SIGCHLD, sigchld_main);
}

void sigint_handler(int sig) {
  sprintf((char *)buffer,"Forced Exit\n");
  write(setupFile,(char *)buffer,strlen((char *)buffer));
  for (int i = 0; i < NCLIENTS; i++) {
    if (client[i].pid!=0) {
      close(client[i].socket);
    }
  }
  close(setupFile);
  close(socketfd);
  unlink("a.out");
  unlink("monitor");
  rmdir("Real");
  exit(0);
}

int show(char query[256],int all){
  MYSQL *con = mysql_init(NULL);
  if (con == NULL){   return finish_with_error(con);  }
  if (mysql_real_connect(con, "localhost", "lti2", "lti2", "monitorizacao_fisica", 0, NULL, CLIENT_MULTI_STATEMENTS) == NULL){  return finish_with_error(con);  }
  if (mysql_query(con, query)) {  return finish_with_error(con); }

  MYSQL_RES *result = mysql_store_result(con);
  if (result == NULL){  return finish_with_error(con); }

  int num_fields = mysql_num_fields(result);
  MYSQL_ROW row;
  MYSQL_FIELD *field;

  if (all==0) {
    while ((row = mysql_fetch_row(result))){
      for(int i = 0; i < num_fields; i++) {
        printf("->%s ", row[i] ? row[i] : "NULL");
      }
      printf("\n");
    }
  }else if (all==1) {
    while ((row = mysql_fetch_row(result))){
        for(int i = 0; i < num_fields; i++){
            if (i == 0) {
               while((field=mysql_fetch_field(result))){
                 if (!(strcmp(field->name,"Data")&&strcmp(field->name,"Tempo"))) {
                   printf("%s\t\t\t", field->name);
                 }else{
                   printf("%s\t\t", field->name);
                 }
               }
               printf("\n");
            }
            printf("%s\t\t", row[i] ? row[i] : "NULL");
        }
    }
    printf("\n");
  }
  mysql_free_result(result);
  mysql_close(con);
  return 0;
}

int janelaTemporal(){
  int area,op,isu,ano,mes,dia;
  char query[256],option[10],data[20];

  printf("Tabelas disponiveis:\n\n");
  sprintf(query,"SHOW TABLES");
  if(show(query,0)){  return 1;  }
  printf("\nArea->");
  scanf("%d",&area);
  printf("\n1-Comportamento\n2-Valores\n\n->");
  scanf("%d",&op);
  if (op==1) {
    sprintf(option,"Behaviour");
  }else if (op==2) {
    sprintf(option,"Values");
  }else{  return 1; }
  printf("\nISu disponiveis:\n");
  sprintf(query,"SELECT ISu FROM Area_%d_%s GROUP BY ISu",area,option);
  if(show(query,0)){  return 1; }
  printf("\nISu->");
  scanf("%d",&isu );
  printf("\nDatas disponiveis nesta area do respetivo ISu:\n");
  sprintf(query,"SELECT Data FROM Area_%d_%s WHERE ISu = %d GROUP BY Data",area,option,isu);
  if(show(query,0)){  return 1; }
  printf("\nAno->");
  scanf("%d", &ano);
  printf("Mes->");
  scanf("%d", &mes);
  printf("Dia->");
  scanf("%d", &dia);
  sprintf(data,"'%d-%d-%d'",ano,mes,dia);
  printf("\e[1;1H\e[2J");
  sprintf(query,"SELECT * FROM Area_%d_%s WHERE ISu = %d AND Data = %s",area,option,isu,data);
  if(show(query,1)){
    return 1;
  }
  getchar();
  return 0;
}

int main(int argc, char const *argv[]) {
  signal(SIGINT,sigint_handler);
  signal(SIGCHLD, sigchld_main);

  configuration();
  newArea();
  if (!fork()) {
    system("mkdir Real");
    exit(0);
  }
  getchar();
  int op=1;
  while (op!=0) {
    printf("\e[1;1H\e[2J");//clear screen
    printf("1-Monitorizacao em tempo real\n2-Janela temporal\n3-Nova Area\n0-Sair\n");
    scanf("%d",&op);
    switch (op) {
      case 1:
        realTimeM();
        break;
      case 2:
        //janela temporal
        if (janelaTemporal()!=0) {
          printf("Dados inseridos errados.\nAcao abortada.\n");
        }
        getchar();
        break;
      case 3:
        printf("\e[1;1H\e[2J");//clear screen
        newArea();
        getchar();
        break;
      case 0:
        printf("Sair\n(pressione enter)");
        getchar();
        sigint_handler(1);
        //fechar fds
        break;
      default:
        printf("Opção inválida.\n");
    }
  }
  return 0;
}
