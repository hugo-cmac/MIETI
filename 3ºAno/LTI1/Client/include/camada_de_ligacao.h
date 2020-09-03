#ifndef CAMADA_DE_LIGACAO_H
#define CAMADA_DE_LIGACAO_H

	#include <sys/ioctl.h>
	#include <sys/wait.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/time.h>
	#include <termios.h>
	#include <fcntl.h>
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <time.h>
	#include <signal.h>
	#include <errno.h>
	#include <dirent.h>
	#include <poll.h>

	#define _POSIX_SOURCE 1

	#define 	MAXSIZE			205520504
	#define 	DATASIZE 		196
	#define 	FILENSIZE 		45
	#define 	ACKSIZE 		3
	#define 	OPCODEFNP 		0x10
	#define 	OPCODEDP 		0x20
	#define 	OPCODEACK 		0x30
	#define 	OPCODENACK 		0x40
	#define 	OPCODEDEL 		0x40
	#define 	BUFFERSIZE 		512


	typedef union Word{
		unsigned int number;
		unsigned char letter[4];
	}wordS;

	typedef struct fileNamePacket{//4+1+45=50
	unsigned char OpcodeAndSize[4];
	unsigned char FileName[FILENSIZE];
	unsigned char crc;
	}filenS;

	typedef struct dataPacket{//3+1+196=200
	unsigned char OpcodeAndPacket[3];
	unsigned char data[DATASIZE];
	unsigned char crc;
	}dataS;

	typedef struct file{
		int fd;
		char FileName[FILENSIZE];
		unsigned int size;
	}fileS;

	fileS *f;

	unsigned int s;
	char *u;
	int b=0;

	int _timeoutcalc(int serie, int length);
	int _rawmode(char *device);
	int _BaudFlag(int BaudRate);
	int	_recoverconnection();

	void printpacket(unsigned char *p, int l){
		printf("----------------------------\n");
		for (size_t i = 0; i < l; i++) {
			printf("%x ", p[i]);
		}
		printf("\n----------------------------\n");
	}

	int msleep(long msec){
		struct timespec ts;
		int res;

		if (msec < 0){
			errno = EINVAL;
			return -1;
		}

		ts.tv_sec = msec / 1000;
		ts.tv_nsec = (msec % 1000) * 1000000;

    	do {
        	res = nanosleep(&ts, &ts);
    	} while (res && errno == EINTR);

    	return res;
	}

	void loading(unsigned seq){
		int p = ( seq*DATASIZE * 100)/s;
		printf("%d%%|",p );
		for (size_t i = 0; i < p; i++) {
			printf("#");
		}
		for (size_t i = 0; i < 100-p; i++) {
			printf(" ");
		}
		printf("|\n");

	}

	u_int8_t gencrc(u_int8_t *data, size_t len){
		u_int8_t crc = 0xff;
		size_t i, j;
		for (i = 0; i < len; i++) {
			crc ^= data[i];
			for (j = 0; j < 8; j++) {
				if ((crc & 0x80) != 0){
					crc = (u_int8_t)((crc << 1) ^ 0x31);
				}else{
					crc <<= 1;
				}
			}
		}
		return crc;
	}

	void headMaker(int size, unsigned char w[size], unsigned char opcode, int number){
		wordS n;
		n.number = number;
		for (int i = 0; i < size; i++) {
			w[size-1-i] = n.letter[i];
		}
		w[0] = w[0] + opcode;
	}

	int getHead(int size, unsigned char w[size]){
		wordS n;
		w[0] = w[0] & 0x0f;
		for (int i = 0; i < size; i++) {
			n.letter[size-1-i] = w[i];
		}
		return n.number;
	}

	int openPort(char* usb, int baud){
		char device[13];
		b = baud;
		u = usb;

		sprintf(device,"/dev/tty%s",usb);
		if (_rawmode(device) != 0) {
			return -1;
		}

		int serie = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
		if (serie <= 0) {
			perror("Erro na abertura da porta serie");
			return -1;
		}

		struct termios config;

		memset(&config, 0, sizeof(config));
		tcgetattr(serie, &config);

		config.c_iflag &= ~(INLCR | ICRNL);
		config.c_iflag |= IGNPAR | IGNBRK;
		config.c_oflag &= ~(OPOST | ONLCR | OCRNL);
		config.c_cflag &= ~(PARENB | PARODD | CSTOPB | CSIZE );
		config.c_cflag |= CLOCAL | CREAD | CS8;
		config.c_lflag &= ~(ICANON | ISIG | ECHO);

		cfsetospeed(&config, _BaudFlag(baud));//baudrate de output
		cfsetispeed(&config, _BaudFlag(baud));//baudrate de leitura

		// Timeouts configuration
		config.c_cc[VTIME] = 0;//adaptavel
		config.c_cc[VMIN]  = 0;

		// Validate configuration
		if (tcsetattr(serie, TCSANOW, &config) < 0) {
			close(serie);
			perror("Erro na validacao das configuracoes\n");
			return -1;
		}
		tcflush(serie,TCIFLUSH);
		
		return serie;
	}

	int serialRead(int serie, unsigned char* packet, int length){
		int n = 0;
		if (_timeoutcalc(serie,length) == 0) {
			if ((n = read(serie, packet, length))<=0){
				perror("Erro na leitura\n");
				return -1;//
			}
			return n;
		}else{
			return 0;//em timeout
		}
	}

	int serialWrite(int serie, unsigned char* packet, int length){
		int n = 0;
		while((n = write(serie, packet, length)) <= 0){
			printf("Erro de leitura na porta serie\n");
			tcflush(serie,TCIFLUSH);
			close(serie);
			serie = _recoverconnection();
		}
		return n;
	}

	void serialClose(int serie) {
		tcdrain(serie);
		close(serie);
	}

	void closefds(int serial) {
		serialClose(serial);
		close(f->fd);
		free(f);
	}

	int _timeoutcalc(int serie,int length){
		int bytes = 0, c = 0;
		clock_t t = clock();
		float timeout = (2*((float) sizeof(dataS)/(b / 8)));

		while(bytes != length){
			c = ioctl(serie, FIONREAD, &bytes);
			if (timeout <= (((float)(clock()-t))/CLOCKS_PER_SEC)) {
				if (bytes != 0) {
					system("clear");
					printf("Trying to reconnect...\n");
					close(serie);
					serie = openPort(u,b);
				}
				return -1;
			}else if(c==-1){
				close(serie);
				serie = _recoverconnection();
			}
		}
		return 0;
	}

	int _rawmode(char *device){
		int status = -1, exitvalue=-1;
		if (exitvalue!=0) {
			if (fork() == 0) {
					if (execlp("stty","stty","-F",device,"raw",NULL)<0) {
						perror("Abortar: Erro no exec\n");
						exit(-1);
					}
			}
			wait(&status);
			exitvalue = WEXITSTATUS(status);
		}
		return exitvalue;
	}

	int _BaudFlag(int BaudRate){
		switch(BaudRate){
			case 50:      return B50; break;
			case 110:     return B110; break;
			case 134:     return B134; break;
			case 150:     return B150; break;
			case 200:     return B200; break;
			case 300:     return B300; break;
			case 600:     return B600; break;
			case 1200:    return B1200; break;
			case 1800:    return B1800; break;
			case 2400:    return B2400; break;
			case 4800:    return B4800; break;
			case 9600:    return B9600; break;
			case 19200:   return B19200; break;
			case 38400:   return B38400; break;
			case 57600:   return B57600; break;
			case 115200:  return B115200; break;
			case 230400:  return B230400; break;
			default : perror("Baudrate inválido\n"); exit(0); break;
		}
	}

	int	_recoverconnection(){
		int s=0;
		for (int i = 1; i < 10; i++) {
			system("clear");
			printf("Trying to reconnect\n%ds\n",i);
			sleep(1);
			if ((s=openPort(u,b))!=-1) {
				return s;
			}
		}
		printf("Communication aborted\n");
		closefds(3);
		exit(0);
	}
	
#endif