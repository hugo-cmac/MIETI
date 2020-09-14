#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <poll.h>
#include <netdb.h>
#include <ifaddrs.h>

#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rsa.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/utsname.h>

#include <linux/if_packet.h>
#include <linux/if_link.h>
#include <linux/if_ether.h>

#include <net/if.h>
#include <net/ethernet.h>

#include <netinet/in.h>
#include <netinet/ip.h>

#include <arpa/inet.h>

#define 	DMB		2097152 //2mb
#define		QKB 	4096 	//4kb
#define		UKB 	1024 	//1kb
#define 	MAX		100
#define		PAY		1470	//Payload Size
#define 	PAK		1472	//Packet Size

typedef struct Limits{
	unsigned short avg;
	unsigned short off;
	unsigned char minavgBit;
	unsigned char minoffBit;
	unsigned char maxavgBit;
	unsigned char maxoffBit;
}limits;

typedef struct Delta{
	unsigned char avgNegative;
	unsigned char offNegative;
	unsigned char avgFloat;
	unsigned char offFloat;
	unsigned short avg;
	unsigned short off;
}deltaSeq;

int son = 0;
int python = 0;

void sigInt_handler(int signum){
	if (python != 0)
		kill(SIGINT, python);
	exit(0);
}

void sigchld_handlerF(int signum){//sons count
    int pid, status;    
    pid = wait(&status);
}

void sigchld_handlerS(int signum){//sons count
    int pid, status;    
    pid = wait(&status);
    if (pid == son)
		son = 0;
}

void infoPacket(int fd, unsigned char *packet){// Tamanho 13bytes
	unsigned char info[36] = {0}, *data = "data", f = 0;

	if (read(fd, info, 36) < 0){
		perror("Erro: Leitura 1 da musica.");
		exit(0);
	}
	
	*packet++ = 0;
	*packet++ = info[22];//n channel
	*packet++ = info[23];//n channel
	int m = info[23]<<8 | info[22];
	
	*packet++ = info[24];//sample rate
	*packet++ = info[25];//sample rate
	*packet++ = info[26];//sample rate
	*packet++ = info[27];//sample rate
	m = info[27]<<24 | info[26]<<16 | info[25]<<8 | info[24];

	*packet++ = info[34];//bits
	*packet++ = info[35];//bits
	m = info[35]<<8 | info[34];

	while (read(fd, info, 1)){
		if (*data == *info){
			*data++;
			f += 1;
		}else if(f > 0){
			*data--;
			f = 0;
		}
		if (f == 4)
			break;
	}

	if (read(fd, info, 4) < 0){
		perror("Erro: Leitura 2 da musica.");
		exit(0);
	}
	*packet++ = info[0];
	*packet++ = info[1];
	*packet++ = info[2];
	*packet++ = info[3];
	m = info[3]<<24 | info[2]<<16 | info[1]<<8 | info[0];
}

int bit(unsigned short v){
	int b = 3;
	while (v ^ 0){
		v = v >> 1;
		b++;
	}
	return b;
}

int power(int p){
	int v = 2;
	while (--p)
		v *= 2;
	return (v-1);
}

unsigned int deltaCalc(unsigned char *music, deltaSeq *delta, limits *size, int bytes){
	short left		= 0,
		  right		= 0,
		  avgMax	= 0,
		  offMax	= 0;
	float avg 		= 0, 
		  off 		= 0,
		  avgB 		= 0,
		  offB 		= 0,
		  temp 		= 0;
	unsigned int i = 0, bavg = 0, boff = 0, n = 0;
	
	while (i < bytes){
		left   	= ((short) music[i++]) + ((short) music[i++]<<8);
		right	= ((short) music[i++]) + ((short) music[i++]<<8);

		avg  	= (float)(left + right)/2;
		off 	= (float)left - avg;
		//-----------Delta-AVG-----------------
		temp = avg - avgB;
		if ((temp - (int)temp) != 0)
			delta[n].avgFloat = 2; 
		else
			delta[n].avgFloat = 1;

		if (temp < 0) {
			delta[n].avgNegative = 2;
			delta[n].avg = (int)-temp;
		}else{
			delta[n].avgNegative = 1;
			delta[n].avg = (int)temp;
		}
		if (delta[n].avg > avgMax)
			avgMax = delta[n].avg;
		bavg += delta[n].avg;
		//-------------------------------------
		
		//-----------Delta-OFF-----------------
		temp = off - offB;
		if ((temp - (int)temp) != 0)
			delta[n].offFloat = 2;
		else
			delta[n].offFloat = 1;
		
		if (temp < 0) {
			delta[n].offNegative = 2;
			delta[n].off = (int)-temp;
		}else{
			delta[n].offNegative = 1;
			delta[n].off = (int)temp;
		}
		if (delta[n].off > offMax)
			offMax = delta[n].off;
		boff += delta[n].off;
		//-------------------------------------
		
		n++;
		avgB  	= avg;
		offB 	= off;
	}
	size->minavgBit = bit(bavg>>9);
	size->avg = power(size->minavgBit - 2);
	
	size->minoffBit = bit(boff>>9);
	size->off = power(size->minoffBit - 2);

	size->maxavgBit = bit(avgMax);
	size->maxoffBit = bit(offMax);
	return bytes;
}

void printBytes(unsigned char *stream, int size){
	for (int i = 0; i < size; i++){
		printf("%.2x.", stream[i]);
	}
	printf("\n");
}

void printBinary(unsigned char *stream, int size) {
  	for (int i = 0; i < size; i++) {
		for (int b = 0; b < 8; b++){
			if((stream[i] >> (7-b)) & 1)
				printf("1");
			else
				printf("0");
		}
		getchar();
  	}
	printf("\n");
}

void streamPacket(unsigned char* dest, unsigned short seq, unsigned char* src, int size){
	short i = 0;
	*dest = 1<<6;
	*dest++ |= seq>>8;
	*dest++ = 0xff & seq;
	while (i++ != size)
		*dest++ = *src++;
}

void laststreamPacket(unsigned char* dest, unsigned short seq, unsigned char* src, int size){
	short i = 0;
	*dest = 2<<6;
	*dest++ |= seq>>8;
	*dest++ = 0xff & seq;
	while (i++ != size)
		*dest++ = *src++;
}

int bitfilling(unsigned char *bytes, unsigned char **left, unsigned char compl, unsigned char decim, int value, int size){
	unsigned short i = 0;
	if (compl){
		size--;	
		**left -= 1;
		if (compl == 2)
			bytes[i] |= 1<<**left;
		if (!**left){
			**left = 8;	
			i++;
		}
	}
	if (decim){
		size--;
		**left -= 1;
		if (decim == 2)
			bytes[i] |= 1<<**left;
		if (!**left){
			**left = 8;	
			i++;
		}
	}
	while (**left <= size){
		size -= **left;
		**left = 8;
		bytes[i] |= (value>>size);
		i++;
	}
	if (size){
		**left -= size;
		bytes[i] |= value<<**left;
	}
	return i;
}

int buildStream(unsigned char *zip, unsigned char *bits, deltaSeq *delta, limits size){
	unsigned short i = 0;
	int byte = 0;

	byte += bitfilling(&zip[byte], &bits, 0, 0, size.minavgBit, 5);
	byte += bitfilling(&zip[byte], &bits, 0, 0, size.minoffBit, 5);
	byte += bitfilling(&zip[byte], &bits, 0, 0, size.maxavgBit, 5);
	byte += bitfilling(&zip[byte], &bits, 0, 0, size.maxoffBit, 5);
	
	while (i < 1024){	
		if ((delta[i].avg <= size.avg) & (delta[i].off <= size.off)) {
			byte += bitfilling(&zip[byte], &bits, 0, 0, 0, 1);
			byte += bitfilling(&zip[byte], &bits, delta[i].avgNegative, delta[i].avgFloat, delta[i].avg, size.minavgBit);
			byte += bitfilling(&zip[byte], &bits, delta[i].offNegative, delta[i].offFloat, delta[i].off, size.minoffBit);
		} else {
			byte += bitfilling(&zip[byte], &bits, 0, 0, 1, 1);
			byte += bitfilling(&zip[byte], &bits, delta[i].avgNegative, delta[i].avgFloat, delta[i].avg, size.maxavgBit);
			byte += bitfilling(&zip[byte], &bits, delta[i].offNegative, delta[i].offFloat, delta[i].off, size.maxoffBit);
		}
		i++;
	}
	return byte;
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

int retransmition(int udp,struct sockaddr_in dest, int last, int size, unsigned char packets[1500][PAY]){
	unsigned char packet[PAK] = {0}, dgram[PAK] = {0};
	while ((packet[0]>>6) != 3)
		recv(udp, packet, PAK, 0);
	//printBytes(packet, 15);
	int i = 0, p = 1;
	while (1){
		i = packet[p++]<<8;
		i |= packet[p++];
		if (i == 1500)
			break;
		else if (last && i == last){
			laststreamPacket(dgram, i, packets[i], size);
			sendto(udp, dgram, PAK, 0, (const struct sockaddr *) &dest, sizeof(dest));
			break;
		}else{
			streamPacket(dgram, i, packets[i], PAY);
			sendto(udp, dgram, PAK, 0, (const struct sockaddr *) &dest, sizeof(dest));
		}
	}
	
	if (p == 3)
		return 0;
	return -(p/2 - 1);
}

void sendSong(int udp, struct sockaddr_in dest, char *file){
	int f = open(file, O_RDONLY);
	if (f < 0){
		perror("Erro: Na abertura da musica.");
		exit(0);
	}
	unsigned char music[DMB] = {0} , packet[1500][PAY] = {0};
	unsigned char *zip = (unsigned char *)packet;

	while (recv(udp, packet[0], PAK, MSG_DONTWAIT) > 0);
	infoPacket(f, zip);
	sendto(udp, packet[0], PAK, 0, (const struct sockaddr *) &dest, sizeof(dest));
	
	limits size = {0};
	deltaSeq delta[UKB] = {0};
	unsigned char bit = 0, dgram[PAK] = {0};
	unsigned int red = 0, bytes = 0, written = 0, i;
	int last = MAX;
	

	//11,88 segundos 
	while ((bytes = read(f, music, DMB))){	
		red = written = 0;
		bit = 8;
		memset(zip, 0, sizeof(packet));
		while (red < bytes){
			if ((bytes - red) < QKB)
				red += deltaCalc(&music[red], delta, &size, bytes - red);
			else
				red += deltaCalc(&music[red], delta, &size, QKB);
			written += buildStream(&zip[written], &bit, delta, size);
		}

		i = 0;
		last = MAX;
		while (written > PAY){
			written -= PAY;
			streamPacket(dgram, i, packet[i], PAY);
			sendto(udp, dgram, PAK, 0, (const struct sockaddr *) &dest, sizeof(dest));
			if (i == last)
				last += retransmition(udp, dest, 0, 0, packet) + MAX;
			i++;

		}
		//printf("%d < %d\n", written, PAY);
		laststreamPacket(dgram, i, packet[i], written);
		sendto(udp, dgram, written+2, 0, (const struct sockaddr *) &dest, sizeof(dest));	
		//retransmition(udp, dest, i, written, packet);
		retransmition(udp, dest, i, written, packet);
		//printf("Last: %d\n\n", i);
	}
	printf("End\n");
	exit(0);
}

int setMulticastSocket(int family, int port, char *multiCastAddr, int loopback, int ttl){
    // Open Socket
    int s = socket(family, SOCK_DGRAM, 0);
    if (s < 0){
        perror("Error on openning Socket descriptor");
        return -1;
    }

    //Socket binding to port
    struct sockaddr_in addr = {family, htons(port)};
    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        perror("Error on port binding");
        return -1;
    }

	int size = 294400;
	if(setsockopt(s, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) < 0){
        perror("Error on adding group");
        return -1;
    }

    //Add group
    struct ip_mreq grp = {0};
    inet_pton(AF_INET, multiCastAddr, &grp.imr_multiaddr);
    if(setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &grp, sizeof(grp)) < 0){
        perror("Error on adding group");
        return -1;
    }

    if(setsockopt(s, IPPROTO_IP , IP_MULTICAST_LOOP, &loopback, sizeof(loopback)) < 0){
        perror("Error on loopback setting");
        return -1;
    }

    if(setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0){
        perror("Error on hops setting");
        return -1;
    }

    return s;
}

int tcpServerSocket(int family, int port){
	int s = socket(family, SOCK_STREAM, 0);
    if (s < 0){
        perror("Error on openning Socket descriptor");
        return -1;
    }

	//Socket binding to port
    struct sockaddr_in addr = {family, htons(port)};
    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        perror("Error on port binding");
        return -1;
    }

	if (listen(s, 5) < 0){
        perror("Error on listen");
        return -1;
    }
	return s;
}

void client(int tcp, char *multicastaddr){
	signal(SIGCHLD, sigchld_handlerS);
	unsigned char buffer [10000] = {0};
	unsigned int bufferSize = 0;
	int f = 0;

	int udp = setMulticastSocket(AF_INET, 9999, multicastaddr, 1, 1);
	struct sockaddr_in dest = {AF_INET, htons(8888)};
    inet_pton(AF_INET, multicastaddr, &dest.sin_addr);

	send(tcp, multicastaddr, 16, 0);
	while ((bufferSize = recv(tcp, buffer, 10000, 0)) > 0){
		switch (buffer[0]){
			case 0:
				//update
				printf("Update\n");
				f = open("music.list", O_RDONLY);\
				bufferSize = read(f, buffer, 10000);
				send(tcp, buffer, bufferSize, 0);
				close(f);
				break;
			case 1:
				//play
				printf("Play %s\n", &buffer[1]);
				if ((son = fork()) == 0)
					sendSong(udp, dest, &buffer[1]);
				break;
			case 2:
				if (son != 0){
					printf("Stop\n");
					kill(son, SIGINT);
				}
				break;	
			case 3: 
				printf("Bye\n");
				if (son != 0)	kill(son, SIGINT);
				return;
		}
	}
	if (son != 0)	kill(son, SIGINT);
	printf("Bye\n");
}

int main(int argc, char const *argv[]){
	signal(SIGINT, sigInt_handler);
	signal(SIGCHLD, sigchld_handlerF);

	python = fork();
	if (python == 0) {
        if (execlp("python3", "python3", "drop.py", NULL) < 0) {
            perror("Erro no Python Script");
            exit(-1);
        }
	}
	char multicastaddr[16] = {0};
	int ls = 189;
	int tcp = tcpServerSocket(AF_INET, 9999);

	while (1){
		struct sockaddr_in clientAddr = {0};
		uint32_t clientLen = sizeof(clientAddr);
		int clientSocket = accept(tcp, (struct sockaddr *)&clientAddr, (socklen_t*)&clientLen);
		if (clientSocket < 0){
			close(clientSocket);
		} else {
			if (!fork()) {
				close(tcp);
				sprintf(multicastaddr, "224.0.0.%d", ls);
				printf("Client on: %s\n", multicastaddr);
				client(clientSocket, multicastaddr);
				close(clientSocket);
				exit(0);
			}
			if (++ls == 256)
				ls = 189;	
		}
	}
	return 0;
}
//59945304
//92847804


//<4bit nr pacote> < 5bit min avgbit > < 5bit min offbit > < 5bit max avgbit > < 5bit max offbit >


