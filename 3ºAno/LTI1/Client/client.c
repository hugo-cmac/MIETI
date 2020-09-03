#include "include/camada_de_ligacao.h"
#include "include/receber_ficheiro.h"
#include "include/enviar_ficheiro.h"

#include <dirent.h>

char *fifo = "fifo";
int leave = 0;
int py = 0;

void handler(int s){
    if (leave==1){
        unlink(fifo);
        unlink("client");
        kill(SIGKILL, py);
        wait(NULL);
        exit(0);
    }else{
        leave++;
    }
}

ssize_t readln(int fildes, void *buf){
	int i = 0;
	while((read(fildes, buf+i, 1 )) != 0) {                                         
	    if(*(char*)(buf+i) == '\n'){            
            i++;
            break;
		}
		i++;
	}
	return i;
}

void python(){
	if ((py = fork()) == 0) {
		if (execlp("python3", "python3", "fileCatcher.py", NULL)<0) {
			perror("Abortar: Erro no exec\n");
			exit(-1);
		}
	}
}

int main(int argc, char const *argv[]){
    signal(SIGINT, handler);

    char b[BUFFERSIZE]; memset(b,'\0', BUFFERSIZE);

    mkfifo( fifo, 0777);
    
    struct pollfd fds[2];

    python();
    
    while (1){
        for (size_t i = 0; i < 2; i++){
            memset(&fds[i], 0, sizeof(struct pollfd));
            if (i==0){
                fds[i].fd = open( fifo, O_RDONLY | O_NONBLOCK);
            }else{
                fds[i].fd = openPort( "USB0", 9600);
            }
            fds[i].events = POLLIN;
        }
        if ( poll(fds, 2, -1) == -1) {
            perror ("Poll error\n");
            return 1;
        }
        if (fds[0].revents & POLLIN){
            int n = readln(fds[0].fd, b);
            if (b[1] == 'a' || b[1] == 'u'){
                write(1, "send\n", 5);

                b[n-1] = '\0';
                sendfile( fds[1].fd, &b[3]);
            }else if (b[1] == 'd'){
                write(1, "delete\n", 7);
                write( 1, &b[1], 1);
            }
        }else if (fds[1].revents & POLLIN){
            write(1, "receive\n", 8);
            receiveFile(fds[1].fd);
            close(fds[1].fd);
        }
        close(fds[0].fd);
    }
    return 0;
}

