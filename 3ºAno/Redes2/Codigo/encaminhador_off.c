#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>


#define MAX 65536
#define INT 5

typedef struct NetAddress{
	unsigned int ipS;
	unsigned int ipE;
	int mask;
	char interface[5];
  struct in_addr ip;
}NetAddress;
typedef struct ifAddress{
	char ip[16];
	char interface[5];
}ifAddress;

struct NetAddress table[INT];
struct ifAddress iface[INT];

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
unsigned int ipDeduction(char ip[16]){
	int ad[4];
	sscanf(ip,"%d.%d.%d.%d",&ad[0],&ad[1],&ad[2],&ad[3]);
 	return ((int)ad[0]*pow(2,24)+(int)ad[1]*pow(2,16)+(int)ad[2]*pow(2,8)+(int)ad[3]*1);
}
unsigned int p(unsigned int b,int pi){
	if(pi==0){
		return 1;
	}else{
		return b*p(b,pi-1);
	}
}
unsigned int maskOn(unsigned int ip,int mask){
	int b=32-mask;
	return (ip-1+1*p(2,b));
}
int routingTable(char ip[16], int mask, char interface[5], int i){
	if (!(mask>32 && mask<0)) {
		memset(&table[i],0,sizeof(table[i]));
		strcpy(table[i].interface,interface);
		inet_aton(ip, &table[i].ip);
		table[i].ipS=ipDeduction(ip);
		table[i].mask=mask;
		table[i].ipE=maskOn(table[i].ipS,mask);
		return 0;
	}else{
		return -1;
	}
}
int readFile(){
  	char line[32], ip[16],interface[5];
  	int fd,mask,i;
  	if ((fd=open("routingTable.txt",O_RDONLY))<0) {
      		perror("Error reading config file\n");
      		return -1;
  	}
  	for ( i = 0; i < INT; i++) {
      		readln(fd,line,1);
      		if (sscanf(line,"%s /%d %s\n",ip,&mask,interface)<0) {
      			perror("Erro na extraçao de informaçao do ficheiro.\n");
			return -1;
      		}
		if (routingTable(ip,mask,interface,i)<0){
			perror("mascara invalida\n");
			return -1;
		}
  	}
  	close(fd);
  	return 0;
}
int rawSocket(int sck){
  	if ((sck = socket (AF_PACKET, SOCK_RAW, htons(ETH_P_IP))) < 0) {
    		perror ("socket");
    		return -1;
  	}
  	return sck;
}
void sigchld_handler(int sig){
	int status;
	wait(&status);
}
int findInt(struct iphdr * ip){
	char ipAd[16];
	int i;
	struct sockaddr_in dest;
	dest.sin_addr.s_addr = ip->daddr;
	sprintf(ipAd,"%s",inet_ntoa(dest.sin_addr));
	for (i = 0; i < INT; i++) {
		if (!strcmp(ipAd,iface[i].ip)) {
			exit(0);
		}
	}
  	for (i = 0; i < INT; i++) {
		if (ipDeduction(ipAd)>table[i].ipS && ipDeduction(ipAd)<table[i].ipE ) {
			printf("Destino:%s pertence a %s/%d.\n", ipAd, inet_ntoa(table[i].ip),table[i].mask);
			printf("E envia por %s.\n", table[i].interface);
			return i;
		}
  	}
	return -1;
}
int sendMsg(unsigned char *buffer){
  	int sendSock,n,on=1;
  	struct sockaddr_in dest;
  	socklen_t len;
  	if ((sendSock = socket (AF_INET, SOCK_RAW, htons(ETH_P_IP))) < 0) {
    		perror ("socket.\n");
    		return -1;
  	}
	struct iphdr *ip = (struct iphdr *)(buffer);
	int index=findInt(ip);
	if (index==-1) {
		return -1;
	}
  	if (setsockopt (sendSock, SOL_SOCKET, SO_BINDTODEVICE, table[index].interface, 4) < 0) {
    		perror ("bind to interface.\n");
    		return -1;
  	}
  	if (setsockopt (sendSock, 0, IP_HDRINCL, &on, sizeof(on)) < 0) {
    		perror ("ip header including error.\n");
    		return -1;
  	}

  	memset(&dest,0,sizeof(dest));
  	dest.sin_family = AF_INET;
  	dest.sin_addr.s_addr = ip->daddr;
  	n=ntohs(ip->tot_len);
  	len=sizeof(dest);
 	printf("enviou\n");
  	if (sendto(sendSock,buffer,n,0,(struct sockaddr *)&dest,len)<0) {
    		perror("error in sendto");
    		exit(-1);
  	}
  	close(sendSock);
  	return 0;
}
void listenTraffic(int sck){
	signal(SIGCHLD, sigchld_handler);
	int n;
	struct sockaddr_in from;
  	unsigned char *buffer = (unsigned char *) malloc(MAX);
  	socklen_t len=sizeof(from);
	printf("Listening\n");
	while (1) {
    		memset(&from,0,sizeof(from)); memset(buffer,0,MAX);
    		if ((n=recvfrom(sck,buffer,MAX,0,(struct sockaddr*)&from,&len))<0) {
      			perror("error in recving msg\n");
      			exit(-1);
    		}
    		buffer = (unsigned char *)(buffer + sizeof(struct ethhdr));
		if (!fork()) {
			if(sendMsg(buffer)<0){
	        		perror("error in sending msg\n");
	        		exit(-1);
	    		}
			exit(0);
		}
  	}
}
void interfaceIp(int sck){
	struct ifreq req;
	int i;
	for (i = 0; i < INT; i++) {
		memset(&req, 0, sizeof(req));
		strcpy(iface[i].interface,table[i].interface);
	 	 strncpy(req.ifr_name, iface[i].interface, IF_NAMESIZE - 1);
		ioctl(sck, SIOCGIFADDR, &req);
		sprintf(iface[i].ip,"%s",inet_ntoa(((struct sockaddr_in *)&req.ifr_addr)->sin_addr));
	}
}

int main(int argc, char const *argv[]) {
	int sck=-1,status,pid=0;
	if (readFile()<0) {
    		perror("file read error");
    		exit(-1);
  	}
	if ((sck = rawSocket(sck)) < 0) {
		perror("raw socket error");
		exit(-1);
	}
	interfaceIp(sck);
	if ((pid=fork())==0) {
		listenTraffic(sck);
	}

	close(sck);

	while (1) {
		int p = wait(&status);
		printf("ERROR ROUTER OFFLINE\n");
		if (p==pid) {
			if ((pid=fork())==0) {
				if ((sck = rawSocket(sck)) < 0) {
					perror("sck");
					exit(-1);
				}
				listenTraffic(sck);
			}
			printf("ROUTER BACK ONLINE\n");
		}
	}

	return 0;
}
