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



int main(int argc, char const *argv[]) {
  char fifoName[15],Buffer[100];
  int fd,n;
  sprintf(fifoName,"Real/ISu_%d",atoi(argv[1]));
  if ((fd = open(fifoName,O_RDONLY))==-1) {
    perror("Sujeito inexistente");
    exit(-1);
  }

  while ((n=read(fd,Buffer,sizeof(Buffer)))>0) {
    write(1,Buffer,n);
  }
  return 0;
}
