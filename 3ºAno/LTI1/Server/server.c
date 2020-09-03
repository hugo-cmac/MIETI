#include "include/camada_de_ligacao.h"
#include "include/receber_ficheiro.h"
#include "include/enviar_ficheiro.h"


char *toWeb = "toWeb";
char *fromWeb = "fromWeb";
int web;
int py = 0;

void handler(int s){
        unlink(toWeb);
        unlink(fromWeb);
        unlink("server");
        kill(SIGKILL, py);
        wait(NULL);
        exit(0);
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
    	if (execlp("python3", "python3", "weblink.py", NULL)<0) {
			perror("Abortar: Erro no exec\n");
			exit(-1);
		}
	}
}

int main(int argc, char const *argv[]){
    umask(0011);
    signal(SIGINT, handler);

    char b[BUFFERSIZE]; memset(b,'\0', BUFFERSIZE);

    mkfifo( toWeb, 0666);
    mkfifo( fromWeb, 0666);

    struct pollfd fds[2];
    int bytes = 0;
    python();
    fds[0].fd = open( fromWeb, O_RDONLY | O_NONBLOCK);
    web = open(toWeb, O_WRONLY);

    while (1){
        for (size_t i = 0; i < 2; i++){
            fds[i].revents = 0;
            fds[i].events = POLLIN;
            if (i == 0){
                ioctl(fds[i].fd, FIONREAD, &bytes);
                if (bytes == 0){
                    close(fds[i].fd);
                    fds[i].fd = open( fromWeb, O_RDONLY | O_NONBLOCK);
                }
            }else{
                fds[i].fd = openPort( "USB1", 9600);
            }
        }
        if ( poll(fds, 2, -1) == -1) {
            perror ("poll");
            return 1;
        }
        if (fds[0].revents & POLLIN){
            int n = readln(fds[0].fd, b);
            b[n-1] = '\0';
            if (b[1] == 'a' || b[1] == 'u'){
                write(1, "Send:\n", 6);
                sendfile( fds[1].fd, &b[3]);
            }else if (b[1] == 'd'){
                write(1, "Delete:\n", 8);
                deletefile(fds[1].fd, &b[3]);
            }
        }else if (fds[1].revents & POLLIN){
            write(1, "Receive:\n", 9);
            receiveFile(fds[1].fd);
        }
        write(1, "End\n", 4);
    }
    return 0;
}

