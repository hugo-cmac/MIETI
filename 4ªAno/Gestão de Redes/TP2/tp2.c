#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define SIZE  2000
#define CMD   400

typedef struct Process{
  int pid;
  char name[CMD];
  char comand[CMD];
  float cpu;
  float memory;
}pidS;

char result[SIZE];
char ipport[25];

ssize_t readln(int fildes, void *buf, ssize_t nbyte){
	int i = 0;
	while((read(fildes, buf+i, nbyte )) != 0){
		i++;
	}
	return i;
}

int exec(char *c, char *p1, char *p2, char *p3, char *p4){
  int status = -1, exitvalue=-1, p[2];
  pipe(p);
	if (exitvalue!=0) {
		if (fork() == 0) {
        dup2(p[1],1); close(p[0]);
				if (execlp(c, c, "-v2c",ipport,"-OU","-Os","-Ov","-Oe", p1, p2, p3, p4, NULL)<0) {
					perror("Abortar: Erro no exec\n");
					exit(-1);
				}
		}
    close(p[1]);
		wait(&status);
		exitvalue = WEXITSTATUS(status);
	}
  memset(result, 0, SIZE); readln(p[0], result, 1); close(p[0]);
  return exitvalue;
}

int getNumberPids(){
  int size;
  exec("snmpget", "hrSystemProcesses.0", NULL, NULL, NULL);
  sscanf(result,"Gauge32: %d",&size);
	return size;
}

void getPID(int n, pidS t[n]){
  int r, index = -1;
  char *token, *s = "\n", c[CMD] = "hrSWRunIndex";
  while (n != 0) {
    exec("snmpbulkget", "-Cr100", c, NULL, NULL);

    if ((n-100) > 0) { n = n - 100; r = 100;}
    else{ r = n; n = 0;}

    token = strtok(result, s);
    for (int i = 0; i < r; i++) {
      index++;
      sscanf(token,"INTEGER: %d", &t[index].pid);

      token = strtok(NULL, s);
    }
    sprintf(c,"hrSWRunIndex.%d", t[index].pid);
  }
}

void getData(int n, pidS t[n], int cpuTime){
  int r, index = -1;
  char *token[2], *s = "\n";

  char c[4][CMD] = { "hrSWRunName",  "hrSWRunPerfCPU",  "hrSWRunPerfMem", "-Cr99"};
  while (n != 0) {
    exec("snmpbulkget", c[3], c[0], c[1], c[2]);

    if ((n-33) > 0) { n = n - 33; r = 33;}
    else{ r = n; n = 0;}

    token[0] = strtok(result, s);
    for (int i = 0; i < r; i++) {
      index++;

      token[1] = strchr(token[0], 34);
      strcpy(t[index].name, token[1]);

      token[0] = strtok(NULL, s);       token[1] = strchr(token[0], 32);
      t[index].cpu = atoi(token[1]);
      t[index].cpu = (t[index].cpu / cpuTime)* 100 ;

      token[0] = strtok(NULL, s);       token[1] = strchr(token[0], 32);
      t[index].memory = atoi(token[1]);
      t[index].memory = (t[index].memory * 100) / (16141920);

      token[0] = strtok(NULL, s);
    }
    if (n<33) {
      sprintf(c[3],"-Cr%d\n", n*3);
    }
    sprintf(c[0],"hrSWRunName.%d",t[index].pid);  sprintf(c[1],"hrSWRunPerfCPU.%d",t[index].pid);
    sprintf(c[2],"hrSWRunPerfMem.%d",t[index].pid);
  }
}

void sortBy(int n, pidS t[n], int op){
  switch (op) {
    case 1://mem
      for(int x = 0; x < n; x++){
        for(int y = 0; y < n-1; y++){
          if(t[y].memory > t[y+1].memory){
              pidS temp = t[y+1];
              t[y+1] = t[y];
              t[y] = temp;
      }}}
      break;
    case 2://cpu
      for(int x = 0; x < n; x++){
        for(int y = 0; y < n-1; y++){
          if(t[y].cpu > t[y+1].cpu){
              pidS temp = t[y+1];
              t[y+1] = t[y];
              t[y] = temp;
      }}}
      break;
  }
}

void printData(int n, pidS t[n]){
  printf("\n\n\tPID\t%%CPU\t%%MEM\t   NAME\n");
  for (int i = n ; i-- > n-20 ; ) {
    printf("\t%d\t %.1f\t %.1f\t%s\n", t[i].pid, t[i].cpu, t[i].memory, t[i].name);
  }
  printf("\n\n\t(crtl^c = To exit)\n");
}

void getTime(int i){
  time_t rawtime;
  struct tm *timeinfo;
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  if (i==0) {
      printf("\t[HORA] : [%d]:[%d]:[%d]\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    }else{
      printf("\n" );
    }
}

void printArray(int n, pidS t[n]){

  char buf[512], title[64];
  time_t rawtime;
  time ( &rawtime );
  sprintf(title, "history/%ld", rawtime);

  int fd = open(title, O_CREAT | O_WRONLY | O_TRUNC, 0666);
  if (fd<0){
    //vamos ficar sem historico
  }
  n--;

  sprintf(buf,"[");
  write(1,buf,strlen(buf));
  write(fd,buf,strlen(buf));
  memset(buf, '\0', sizeof(buf));
  while (n!=2) {
    sprintf(buf,"{\"pid\":%d,\"cpu\":%.2f,\"memory\":%.2f,\"name\":%s},", t[n].pid, t[n].cpu, t[n].memory, t[n].name);
    write(1,buf,strlen(buf));
    write(fd,buf,strlen(buf));
    memset(buf, '\0', sizeof(buf));
    n--;
  }
  sprintf(buf, "{\"pid\":%d,\"cpu\":%.2f,\"memory\":%.2f,\"name\":%s}]", t[n--].pid, t[n--].cpu, t[n--].memory, t[n--].name);
  write(1,buf,strlen(buf));
  write(fd,buf,strlen(buf));
  close(fd);
}

int getTimeTicks(){
  int t;
  char x[20];
  exec("snmpget", "hrSystemUptime.0", NULL, NULL, NULL);
  sscanf(result,"Timeticks: (%d) %s\n",&t,x);
  return t;
}

void printTerminal() {
  while (1) {
    int npid = getNumberPids();
    int cpuTime = getTimeTicks();
    pidS table[npid];
    getPID(npid, table);
    getData(npid, table, cpuTime);
    sortBy(npid, table, 1);
    system("clear");
    getTime(0);
    printData(npid, table);
    for (int i = 0; i < npid; i++) {
      memset(&table[i], 0, sizeof(pidS));
    }
  }
}

void printWeb() {
  int npid = getNumberPids();
  int cpuTime = getTimeTicks();
  pidS table[npid];
  getPID(npid, table);
  getData(npid, table, cpuTime);
  sortBy(npid, table, 1);
  printArray(npid, table);
}

int main(int argc, char const *argv[]) {
  //.out 127.0.0.1 9999 [1-terminal/2-web]
  if (argc<4 || argc >5) {
    printf("Numero de argumentos errados.\n");
  }else{
    if (atoi(argv[3]) == 1) {
      sprintf(ipport,"%s:%s",argv[1], argv[2]);
      printTerminal();
    }else if (atoi(argv[3]) == 2){
      sprintf(ipport,"%s:%s",argv[1], argv[2]);
      printWeb();
    }
  }
  return 0;
}
