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

#define BUFFERSIZE 420
#define NCLIENTS 10

//Valores standart lidos do ficheiro de configuraçao
int portUDP,paF,nsF,pmF;
int portTCP,area=1;
char ipServerTCP[10];
//Descritor de ficheiro de setup
int setupFile;
//Buffer
unsigned char buffer[BUFFERSIZE];
unsigned char tcpbuffer[BUFFERSIZE];
//Descritor Socket
int socketfd;
int socketTCP;
//id de processo de armazeamento e pipe de comunicação
int sonPID;
int fifo=-1;
char fifoName[32];
//Estrutura de cliente e Sistema Sensor
typedef struct SisteSensor{
  int iSS;
  int iSu;
  int pipe[2];
  int pid;
  struct sockaddr_in SS;
}SistemaSensor;
//declaraçao de servidor e array de clientes

struct SisteSensor clientaddr[NCLIENTS];

int bit32Read(int i){
  int a;
  a = (int)buffer[i]*pow(2,24)+(int)buffer[i+1]*pow(2,16)+(int)buffer[i+2]*pow(2,8)+(int)buffer[i+3]*1;
  return a;
}

void bit32Input(int begin,int end,int var,char b[BUFFERSIZE]){
  //memcpy(dst, src, 4);
  int n=24;
  for (int i = begin; i < end; i++) {
    b[i]=var >> n;
    n=n-8;
  }
}

void timeStruct(time_t ts,char timeStamp[20]){
  struct tm * t;
  t = localtime((const time_t *) &ts);
  sprintf(timeStamp,"%d/%d/%d %d:%d:%d", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
}

void configuration(){
  int configfd;
  memset(buffer,'\0',BUFFERSIZE);
  if ((setupFile = open("setupUDPserver.txt",O_CREAT | O_WRONLY | O_TRUNC ,0666))<0) {
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
    exit(-1);//sq sai
  }else{
    read(configfd,(char *)buffer,BUFFERSIZE);
    sscanf((char *)buffer,"%d\n%d\n%d\n%d\n%d\n%s",&portUDP,&paF,&nsF,&pmF,&portTCP,ipServerTCP);
    close(configfd);
    sprintf((char *)buffer,"Config file read.\n");
    write(setupFile,(char *)buffer,strlen((char *)buffer));
  }
  memset(buffer,'\0',BUFFERSIZE);
  if ( (socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0 ) {
		perror("UDP socket error");
    sprintf((char *)buffer,"UDP socket error.\n");
    write(setupFile,(char *)buffer,strlen((char *)buffer));
    exit(-1);
	}else{
    sprintf((char *)buffer,"UDP socket file descriptor created.\n");
    write(setupFile,(char *)buffer,strlen((char *)buffer));
  }
  memset(buffer,'\0',BUFFERSIZE);
  //Limpeza das Estruturas
  struct sockaddr_in UDPservaddr;
  struct sockaddr_in TCPservaddr;
	memset(&UDPservaddr, 0, sizeof(UDPservaddr));
  memset(&TCPservaddr, 0, sizeof(TCPservaddr));
  for (int i = 0; i < NCLIENTS; i++) {
    memset(&clientaddr[i], 0, sizeof(clientaddr[i]));
  }
  //Configuraçao do servidor UDP
  UDPservaddr.sin_family = AF_INET;      //IPv4
	UDPservaddr.sin_addr.s_addr = INADDR_ANY;
	UDPservaddr.sin_port = htons(portUDP);
  sprintf((char *)buffer,"UDP Server information filled:\n-IPV4;\n-IP Address;\n-PORT NUMBER: %d.\nTCP Server information filled:\n-IPV4;\n-IP Address: %s;\n-PORT NUMBER: %d.\n",portUDP,ipServerTCP,portTCP);
  write(setupFile,(char *)buffer,strlen((char *)buffer));
  memset(buffer,'\0',BUFFERSIZE);
  //Associar servidor á Socket
	if ( bind(socketfd, (struct sockaddr *)&UDPservaddr, sizeof(UDPservaddr)) < 0 ){
		perror("Bind error");
    sprintf((char *)buffer,"Bind error.\n");
    write(setupFile,(char *)buffer,strlen((char *)buffer));
    exit(-1);
	}else{
    sprintf((char *)buffer,"Socket binded with the server address.\n");
    write(setupFile,(char *)buffer,strlen((char *)buffer));
  }
  memset(buffer,'\0',BUFFERSIZE);
  if ( (socketTCP = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		perror("TCP socket error");
    sprintf((char *)buffer,"TCP socket error.\n");
    write(setupFile,(char *)buffer,strlen((char *)buffer));
    exit(-1);
	}else{
    sprintf((char *)buffer,"TCP socket file descriptor created.\n");
    write(setupFile,(char *)buffer,strlen((char *)buffer));
  }
  //Configuraçao do servidor TCP
  TCPservaddr.sin_family = AF_INET;      //IPv4
	TCPservaddr.sin_addr.s_addr = inet_addr(ipServerTCP);
	TCPservaddr.sin_port = htons(portTCP);//porta tcp
  sprintf((char *)buffer,"TCP Server information filled:\n-IPV4;\n-IP Address;\n-PORT NUMBER: %d.\n",portTCP);
  write(setupFile,(char *)buffer,strlen((char *)buffer));

  memset(buffer,'\0',BUFFERSIZE);
  if ( (connect(socketTCP, (struct sockaddr *) &TCPservaddr, sizeof(TCPservaddr))) < 0 ){
		perror("TCP connection with server failed.\n");
    sprintf((char *)buffer,"TCP connection with server failed.\n");
    write(setupFile,(char *)buffer,strlen((char *)buffer));
    exit(-1);
	}else{
    sprintf((char *)buffer,"TCP connection with server successful.\n");
    write(setupFile,(char *)buffer,strlen((char *)buffer));
  }
  bit32Input(0,4,4,(char *)tcpbuffer);
  bit32Input(4,8,area,(char *)tcpbuffer);

  if(send(socketTCP, (char *)tcpbuffer,sizeof(tcpbuffer),0)<0) {
        perror("Send TCP error.\n");
        sprintf((char *)buffer,"Send TCP error.\n");
        write(setupFile,(char *)buffer,strlen((char *)buffer));
        exit(-1);
  }
  memset(tcpbuffer,'\0',BUFFERSIZE);
}

void printSistS(){
  for (int i = 0; i < 10; i++) {
    if (clientaddr[i].iSS!=0) {
      printf("System %d\n", i);
    }
  }
}

void connectSistSens(){
  struct sockaddr_in tempClient;
  memset(&tempClient, 0, sizeof(tempClient));
  socklen_t len=sizeof(tempClient);
  int op=0,tp,iss;

  memset(buffer,'\0',BUFFERSIZE);

  while (op==0) {
    printf("\e[1;1H\e[2J");
    printf("Turn on your Systems:\n");
    printSistS();
    printf("1-Continue\n0-Update\n");
    scanf("%d",&op);
    if (op==0) {
      recvfrom(socketfd, (char *)buffer, BUFFERSIZE, 0, ( struct sockaddr *) &tempClient, &len);
      tp = bit32Read(0);
      if (tp==4) {
        iss = bit32Read(4);
        sprintf((char *)buffer,"System %d-CONNECTED.\n",iss);
        write(setupFile,(char *)buffer,strlen((char *)buffer));
        clientaddr[iss].iSS=iss;
        clientaddr[iss].SS=tempClient;
        //printf("%s \n", inet_ntoa(clientaddr[iss].SS.sin_addr ));
      }
    }else if(op==1){
      op=0;
      for (int i = 0; i < NCLIENTS; i++) {
        if (clientaddr[i].iSS!=0) {
          op=1;
        }
      }
      if (op==0) {
        printf("No Systems connected\n");
        sleep(3);
      }
    }
  }
}

void closefd(){
  close(setupFile);
  close(socketfd);
  close(socketTCP);
}

void sigalrm1_handler(int sig) {
  closefd();
  exit(0);
}

void sigint_recvPacket(int sig) {
  signal(SIGALRM,sigalrm1_handler);
  unlink(fifoName);
  close(fifo);
  alarm(1);
  while (1) {
    recv(socketfd, (char *)buffer, BUFFERSIZE, 0);
  }
}

void sigchld_handler(int sig){
  int status,pid;
  pid=wait(&status);
  for (int i = 0; i < NCLIENTS; i++) {
    if (clientaddr[i].pid==pid) {
      clientaddr[i].pid=0;
    }
  }
}

void errorPacket(int iss,int ts, int er){
  int errorfd;
  char name[25],timeStamp[20];
  timeStruct(ts,timeStamp);
  switch (er) {
    case 0:
      sprintf(name,"Error/erro_0_iss:%d",iss);
      sprintf((char *)buffer,"Time:%s\nType:0-Number of samples above limit\n.",timeStamp);
      break;
    case 1:
      sprintf(name,"Error/erro_1_iss:%d",iss);
      sprintf((char *)buffer,"Time:%s\nType:1-Sample time below limit.\n",timeStamp);
      break;
    case 2:
      sprintf(name,"Error/erro_2_iss:%d",iss);
      sprintf((char *)buffer,"Time:%s\nType:2-Message time below limit.\n",timeStamp);
      break;
    case 3:
      sprintf(name,"Error/erro_3_iss:%d",iss);
      sprintf((char *)buffer,"Time:%s\nType:3-Sample time overflow.\n",timeStamp);
      break;
    case 4:
      sprintf(name,"Error/erro_4_iss:%d",iss);
      sprintf((char *)buffer,"Time:%s\nType:4-Message time overflow.\n",timeStamp);
      break;
    case 5:
      sprintf(name,"Error/erro_5_iss:%d",iss);
      sprintf((char *)buffer,"Time:%s\nType:5-Packet error.\n",timeStamp);
      break;
  }
  if ((errorfd = open(name,O_CREAT | O_WRONLY | O_TRUNC ,0666))==-1) {
    perror("ErrorFile creation error");
    exit(-1);
  }
  write(errorfd,(char *)buffer,strlen((char *)buffer));
  close(errorfd);
}

float adjustNegative(int n){
  int tmp = bit32Read(n);
  if (tmp>510818) {
    tmp=tmp-1200000;
  }
  return ((float)tmp/1000.00);
}

void storagePacket(int iSu,int i){
  char logName[64], timeStamp[20],amostra[8][51];
  int fd,n;
  int tp,iss,pm,pa,ns,ts;
  float acX,acY,acZ,gX,gY,gZ,temp;

  sprintf(logName,"Saved_logs/LOG_ISS-%d_ISU-%d.txt",i,iSu);
	if ((fd = open(logName,O_CREAT | O_WRONLY | O_TRUNC ,0666))==-1) {
    perror("Error creating log file.\n");
    exit(-1);
  }
  memset(buffer,'\0',BUFFERSIZE);memset(logName,'\0',sizeof(logName));
  sprintf(fifoName,"ISSP/ISu_%d",iSu);
  if ((fifo = open(fifoName,O_RDONLY))==-1) {
    perror("erro fifo\n");
    exit(-1);
  }
  while ((n=read(fifo,buffer, BUFFERSIZE))>0) {
    tp = bit32Read(0);
    if (tp == 3) {
      iss = bit32Read(4);
      ts = bit32Read(8);
      int er = bit32Read(12);
      memset(buffer,'\0',BUFFERSIZE);
      errorPacket(iss,ts,er);
      memset(buffer,'\0',BUFFERSIZE);
      printf("(Error message from iSS %d)\n",i);
      sprintf((char *)buffer,"Error message from iSS %d.\n",i);
      write(setupFile,(char *)buffer,strlen((char *)buffer));
      close(fd);
      close(fifo);//diferente e unlink
      unlink(fifoName);
      memset(tcpbuffer,'\0',BUFFERSIZE);
      bit32Input(0,4,tp,(char *)tcpbuffer);
      bit32Input(4,8,iss,(char *)tcpbuffer);
      bit32Input(8,12,iSu,(char *)tcpbuffer);
      bit32Input(12,16,ts,(char *)tcpbuffer);
      if(send(socketTCP, (char *)tcpbuffer,sizeof(tcpbuffer),0)<0) {
            perror("Send TCP error.\n");
            sprintf((char *)buffer,"Send TCP error.\n");
            write(setupFile,(char *)buffer,strlen((char *)buffer));
            exit(-1);
      }
      closefd();
      exit(0);
    }else if(tp==1){
      iss = bit32Read(4);
      ts = bit32Read(8);
      pm = bit32Read(12);
      pa = bit32Read(16);
      ns = bit32Read(20);
      bit32Input(0,4,tp,(char *)tcpbuffer);
      bit32Input(4,8,iss,(char *)tcpbuffer);
      bit32Input(8,12,iSu,(char *)tcpbuffer);
      bit32Input(12,16,ns,(char *)tcpbuffer);
      bit32Input(16,20,ts,(char *)tcpbuffer);
      n=20;
      for (int i = 0; i < ns; i++) {
        memset(amostra[i],'\0',51);
        n=n+4;    acX=adjustNegative(n);
        bit32Input(n-4,n,bit32Read(n),(char *)tcpbuffer);
        //printf("%.2f ",acX);
        n=n+4;    acY=adjustNegative(n);
        bit32Input(n-4,n,bit32Read(n),(char *)tcpbuffer);
        //printf("%.2f ",acY);
        n=n+4;    acZ=adjustNegative(n);
        bit32Input(n-4,n,bit32Read(n),(char *)tcpbuffer);
        //printf("%.2f\n",acZ);
        n=n+4;    gX=adjustNegative(n);
        bit32Input(n-4,n,bit32Read(n),(char *)tcpbuffer);

        n=n+4;    gY=adjustNegative(n);
        bit32Input(n-4,n,bit32Read(n),(char *)tcpbuffer);

        n=n+4;    gZ=adjustNegative(n);
        bit32Input(n-4,n,bit32Read(n),(char *)tcpbuffer);

        n=n+4;    temp=adjustNegative(n);
        bit32Input(n-4,n,bit32Read(n),(char *)tcpbuffer);
        sprintf(amostra[i],"<%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f>",acX,acY,acZ,gX,gY,gZ,temp);
      }
      if(send(socketTCP, (char *)tcpbuffer,sizeof(tcpbuffer),0)<0) {
            perror("Send TCP error.\n");
            sprintf((char *)buffer,"Send TCP error.\n");
            write(setupFile,(char *)buffer,strlen((char *)buffer));
            exit(-1);
      }
      timeStruct(ts, timeStamp);
      sprintf((char *)buffer,"%d;%d;%s;%d;%d;%d;",iss,iSu,timeStamp,pa,ns,pm);
      for (int i = 0; i < ns; i++) {
        strcat((char *)buffer,amostra[i]);
      }
      buffer[strlen((char *)buffer)+1]='\n';
      write(fd,buffer,strlen((char *)buffer)+2);
      memset(buffer,'\0',BUFFERSIZE);
      memset(tcpbuffer,'\0',BUFFERSIZE);
    }
  }
  close(fd);
  close(fifo);//diferente
  closefd();
  exit(0);
}

void sigalrm2_handler(int sig){
  for (int i = 0; i < NCLIENTS; i++) {
    if (getpid()==clientaddr[i].pid) {
      printf("(15sec passed without receiving any message, disconnection assumed on iSS: %d)\n",clientaddr[i].iSS);
      sprintf((char *)buffer,"(15sec passed without receiving any message, disconnection assumed on iSS: %d)\n",clientaddr[i].iSS);
      write(setupFile,(char *)buffer,strlen((char *)buffer));
    }
  }
  close(fifo);//diferente
  closefd();
  exit(-1);
}

void printBuffer() {
  for (int i = 0; i < 50; i++) {
    printf("%u",buffer[i]);
  }
  printf("\n");
}

void recvPacket(int iSu,int i){
  signal(SIGINT,sigint_recvPacket);
  signal(SIGALRM, sigalrm2_handler);
  int tp;
  struct sockaddr_in tempClient;
  memset(&tempClient, 0, sizeof(tempClient));
  socklen_t len=sizeof(tempClient);

  memset(buffer,'\0',BUFFERSIZE);
  if ((sonPID=fork())<0){
    perror("Fork Storage error.\n");
    sprintf((char *)buffer,"Fork Storage error on iSS %d.\n",i);
    write(setupFile,(char *)buffer,strlen((char *)buffer));
    exit(-1);
  }else if (sonPID==0){
    storagePacket(iSu,i);
  }else{
    sprintf(fifoName,"ISSP/ISu_%d",iSu);
    mkfifo(fifoName,0666);
    if ((fifo=open(fifoName, O_WRONLY))<0) {
      perror("error fifo p\n");
      exit(-1);
    }
    while (1) {
      recvfrom(socketfd, (char *)buffer, BUFFERSIZE, MSG_PEEK, ( struct sockaddr *) &tempClient, &len);
      if (strcmp(inet_ntoa(tempClient.sin_addr),inet_ntoa(clientaddr[i].SS.sin_addr))==0) {
        recvfrom(socketfd, (char *)buffer, BUFFERSIZE, 0, ( struct sockaddr *) &tempClient, &len);
        tp=bit32Read(0);
        if (tp==3) {
          write(fifo,buffer,sizeof(buffer));//diferente
          close(fifo);//diferente
          closefd();
          exit(-1);
        }else{
          write(fifo,buffer,sizeof(buffer));
          memset(buffer,'\0',BUFFERSIZE);
        }
        alarm(15);
      }
    }
  }
}

void startPacket(int iss,int iSu){
  int op;
  int tp,pm,pa,ns,ts;
  printf("Config files values:\n->Sample time: %d\n->Number of samples: %d\n->Message time: %d",paF,nsF,pmF);
  printf("\n\nUse config files PA and NS?\n1-Yes\n2-No\n" );
  scanf("%d",&op);
  while (op!=1 && op!=2) {
    printf("Invalid option! Try again\n->");
    scanf("%d",&op);
  }
  if (op==1) {
    pm=pmF;
  	pa=paF;
  	ns=nsF;
  }else if (op==2){

    printf("Type sample time in milis:\n(Min = 25ms)\n");
    scanf("%d",&pa);
    printf("Type number of samples:\n(Max = 8)\n");
    scanf("%d",&ns);
    printf("Type message time:\n(Min = 200ms)\n");
    scanf("%d",&pm);
  }
	tp=0;
  time((time_t *)&ts);
	memset(buffer,'\0',BUFFERSIZE);
  bit32Input(0,4,tp,(char *)buffer);
  bit32Input(4,8,ts,(char *)buffer);
  bit32Input(8,12,pm,(char *)buffer);
  bit32Input(12,16,pa,(char *)buffer);
  bit32Input(16,20,ns,(char *)buffer);

  memset(tcpbuffer,'\0',BUFFERSIZE);
  bit32Input(0,4,tp,(char *)tcpbuffer);
  bit32Input(4,8,iss,(char *)tcpbuffer);
  bit32Input(8,12,iSu,(char *)tcpbuffer);
}

void stopPacket(int pid,int iss, int iSu){
  int tp=2,sr=0;
  kill(pid,SIGINT);
  bit32Input(0,4,tp,(char *)buffer);
  bit32Input(4,8,sr,(char *)buffer);

  memset(tcpbuffer,'\0',BUFFERSIZE);
  bit32Input(0,4,tp,(char *)tcpbuffer);
  bit32Input(4,8,iss,(char *)tcpbuffer);
  bit32Input(8,12,iSu,(char *)tcpbuffer);
}

void sigint_handler(int sig) {
  sprintf((char *)buffer,"Forced Exit\n");
  write(setupFile,(char *)buffer,strlen((char *)buffer));
  for (int i = 0; i < NCLIENTS; i++) {
    if (clientaddr[i].pid!=0) {
      kill(clientaddr[i].pid,SIGINT);
    }
  }
  closefd();
  unlink("concentrador");
  exit(0);
}

int main(int argc, char const *argv[]) {
  signal(SIGINT,sigint_handler);
  int op=1,i,iSu;
  socklen_t len;
  configuration();
  connectSistSens();
  //criaçao de pastas para uma melhor organização
  system("mkdir Error");
  system("mkdir Saved_logs");
  system("mkdir ISSP");
  while (op!=0) {
    printf("\e[1;1H\e[2J");//clear screen
    printf("1-Start\n2-Stop\n3-New device\n0-Leave\n");
		scanf("%d",&op);
    switch (op) {
      case 1:
        printf("Choose System\n(type system number)\n\n");
        printSistS();
        scanf("%d", &i);
        while (clientaddr[i].pid!=0 && clientaddr[i].iSS!=0) {
          printf("System occupied or inexistent\nType 0 to cancel\n");
          scanf("%d", &i);
        }
        if (i!=0) {
          while (clientaddr[i].iSu==0) {
            printf("Type subject identification number:\n");
            scanf("%d", &iSu);
            for (int d = 0; d < NCLIENTS; d++) {
              if (iSu==clientaddr[d].iSu && iSu==0) {
                printf("Invalid subject identification number.\nTry again\n");
              }else{
                clientaddr[i].iSu=iSu;
              }
            }
          }
          if ((clientaddr[i].pid=fork())<0) {
            perror("Fork error");
            sprintf((char *)buffer,"Fork error on iSS %d.\n",clientaddr[i].pid);
            write(setupFile,(char *)buffer,strlen((char *)buffer));
            clientaddr[i].pid=0;
            exit(-1);
          }else if (clientaddr[i].pid==0){
            clientaddr[i].pid=getpid();
            recvPacket(iSu,i);
          }else{
            signal(SIGCHLD, sigchld_handler);
            sprintf((char *)buffer,"Start message sent to System nr %d.\n",i);
            write(setupFile,(char *)buffer,strlen((char *)buffer));
            memset(buffer,'\0',BUFFERSIZE);
            startPacket(i,iSu);
          }
        }
        break;
      case 2:
        printf("Choose System\n(type system number)\n\n");
        printSistS();
        scanf("%d", &i);
        clientaddr[0].pid=1;
        while (clientaddr[i].pid==0) {
          printf("System idle\nType 0 to cancel\n");
          scanf("%d", &i);
        }
        if (i!=0){
          memset(buffer,'\0',BUFFERSIZE);
          sprintf((char *)buffer,"Stop message sent to System nr %d.\n",i);
          write(setupFile,(char *)buffer,strlen((char *)buffer));
          memset(buffer,'\0',BUFFERSIZE);
          stopPacket(clientaddr[i].pid,i,clientaddr[i].iSu);
          clientaddr[i].iSu=0;
          clientaddr[i].pid=0;
        }
        clientaddr[0].pid=0;
        break;
      case 3:
        connectSistSens();
        break;
      case 0:
        printf("Shutdown.\n");
        sprintf((char *)buffer,"Exit SYSTEM\n");
        write(setupFile,(char *)buffer,strlen((char *)buffer));
        closefd();
        break;
      default:
        printf("Invalid option.\n");
    }
    if (op!=0 && i!=0) {
      len=sizeof(clientaddr[i].SS);
      if(sendto(socketfd, buffer,sizeof(buffer), 0, (struct sockaddr *) &clientaddr[i].SS, len)<0){
						perror("Send UDP error.\n");
            sprintf((char *)buffer,"Send UDP error.\n");
            write(setupFile,(char *)buffer,strlen((char *)buffer));
            exit(-1);
			}
      if(send(socketTCP, (char *)tcpbuffer,sizeof(tcpbuffer),0)<0) {
            perror("Send TCP error.\n");
            sprintf((char *)buffer,"Send TCP error.\n");
            write(setupFile,(char *)buffer,strlen((char *)buffer));
            exit(-1);
      }
    }
    memset(buffer,'\0',BUFFERSIZE);
    memset(tcpbuffer,'\0',BUFFERSIZE);
  }
  unlink("concentrador");
  rmdir("ISSP");
  return 0;
}

//ligacao tcp com gestor
