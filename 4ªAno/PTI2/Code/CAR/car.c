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

//http://beej.us/guide/bgnet/html/

#define PACKSIZ 256
#define ALRTSIZ 190
#define HELLSIZ 15
#define UPDTSIZ 4

typedef struct RegistPacket{
    uint8_t id;
    uint8_t ip[16];
    uint8_t registration[6];
    uint8_t date[4];
    uint32_t hash;
    uint8_t modelNowner[160];
}Registpacket;

typedef struct AlertPacket{
    uint8_t id;
    uint16_t alertID;
    uint64_t timestamp;
    uint16_t ttl;
    uint16_t coordX;
    uint16_t coordY;
    uint8_t info[173];
}Alertpacket;

typedef struct HelloPacket{
    uint8_t id;
    uint16_t alertID;
    uint16_t priorityAlert;
    uint32_t hash;
    uint16_t coordX;
    uint16_t coordY;
}Hellopacket;

typedef struct ServicePacket{
    uint8_t id;
    uint8_t servID;
    uint32_t dest;
    uint64_t timestamp;
    uint8_t info[173];
}Servicepacket;

typedef struct Online{
    uint64_t end;
    unsigned short alertID;
    unsigned char priority;
    unsigned short coordX;
    unsigned short coordY;
    char info[173];
    struct Online *next;
}online;

struct timespec ts1 = {0};
unsigned int hashID = 0;
unsigned short lastPacket = 0;
unsigned short lastPriority = 0;
unsigned char priorityPacket[1024] = {0};
unsigned char *packetStack[1024] = {NULL};

char *sessionfile = NULL;

online *onlineStack = NULL;
Servicepacket *serv[2] = {NULL};

RSA *sk = NULL, *pk = NULL;

void moveTo(int row, int col) {
    printf("\x1b[%d;%df", row, col);
}

void clearTerm(){
    printf("\x1b[2J");
    moveTo(4,0);
    printf("-------------------------------------------------------------------------------------------------");
}

void printAlert(online *a){
    struct tm *t = NULL;

    if (serv[0] != NULL) {
        moveTo(1, 0);
        t = localtime((time_t *)&serv[0]->timestamp);
        printf("%s -> Emissao: %02d:%02d:%02d", serv[0]->info, t->tm_hour, t->tm_min, t->tm_sec);
    }

    if (serv[1] != NULL) {
        moveTo(3, 0);
        t = localtime((time_t *)&serv[1]->timestamp);
        printf("%s -> Emissao: %02d:%02d:%02d", serv[1]->info, t->tm_hour, t->tm_min, t->tm_sec);
    }

    for (short i = 0; i < 20; i++) {
        if (i < 10){
            moveTo(5 + i, 0);
        }else{
            moveTo(5 + (i - 10), 45);
            printf("|");
            moveTo(5 + (i - 10), 53);
        }
        if (a != NULL) {
            t = localtime((time_t *)&a->end);
            if (a->priority)
                printf("\x1b[41m%s(%04dx, %04dy) -> Fim: %02d:%02d:%02d\x1b[0m", a->info, a->coordX, a->coordY, t->tm_hour, t->tm_min, t->tm_sec);
            else
                printf("%s(%04dx, %04dy) -> Fim: %02d:%02d:%02d", a->info, a->coordX, a->coordY, t->tm_hour, t->tm_min, t->tm_sec);
            a = a->next;
        }else{
            printf("\x1b[0K");
        }
    }
}

void sigchld_handler(int signum){
    int pid, status;
    while (1){
        pid = waitpid(WAIT_ANY, &status, WNOHANG);
        if (pid < 0)
            break;
        if (pid == 0)
            break;
    }
}

void update(){
    online *temp = onlineStack;
    if (onlineStack->next != NULL)
        onlineStack = onlineStack->next;
    else
        onlineStack = NULL;
    free(packetStack[temp->alertID]);
    packetStack[temp->alertID] = NULL;
    priorityPacket[temp->alertID] = 0;
    free(temp);
}

void setAlarm(int sig){
    if (onlineStack != NULL) {
        unsigned long long t = time(NULL);
        if (t >= onlineStack->end){
            update();
            setAlarm(0);
        }else{
            t = time(NULL);
            t = onlineStack->end - t;
            alarm((int) t);
        }
    }
    if (!fork()){
        printAlert(onlineStack);
        exit(0);
    }
}

void insert(online *temp){
    if (onlineStack == NULL) {//nao ha alertas
        onlineStack = temp;
    }else{
        online *next = onlineStack;
        online *prev = NULL;
        while (next != NULL && next->end <= temp->end){//percorrer alertas
            prev = next;
            next = next->next;
        }
        if (next == NULL) {//final da lista
            prev->next = temp;
        }else{
            if(prev != NULL){//meio da lista
                temp->next = next;
                prev->next = temp;
            }else{//inicio da lista
                temp->next = onlineStack;
                onlineStack = temp;
            }
        }
    }
    setAlarm(0);
}

unsigned int hash(unsigned char *str){
    unsigned int hash = (str[2]-65)*260000 + (str[3]-65)*10000 + (str[0]-48)*1000 + (str[1]-48)*100 + (str[4]-48)*10 + (str[5]-48);
    return hash;
}

void getIfIpaddr(char *interface, int family, unsigned char *ip){
    struct ifaddrs *ifaddr;
    char host[NI_MAXHOST];
    if (getifaddrs(&ifaddr) == -1) {
       perror("Error get interfaces info");
       exit(-1);
    }

    while (ifaddr!=NULL){
        if (ifaddr->ifa_addr->sa_family == family) {
            if (!strcmp(interface, ifaddr->ifa_name)){
                getnameinfo(ifaddr->ifa_addr,
                  (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
                  host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                inet_pton(AF_INET6, host, ip);
                return;
            }
        }
        if (ifaddr->ifa_next == NULL)
            break;
        else
            ifaddr = ifaddr->ifa_next;
    }
    freeifaddrs(ifaddr);
}

int setMulticastSocket(int family, int port, char *multiCastAddr, char *interface, int loopback, int hops){
    int size = 256;
    // Open Socket
    int s = socket(family, SOCK_DGRAM, 0);
    if (s < 0){
        perror("Error on openning Socket descriptor");
        return -1;
    }

    //Socket binding to port
    struct sockaddr_in6 addr = {family, htons(port)};
    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        perror("Error on port binding");
        return -1;
    }

    //Add group
    struct ipv6_mreq grp = {0};
    inet_pton(AF_INET6, multiCastAddr, &grp.ipv6mr_multiaddr);
    if(setsockopt(s, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &grp, sizeof(grp)) < 0){
        perror("Error on adding group");
        return -1;
    }

    if(setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, interface, 4) < 0){
        perror("Error on interface binding");
        return -1;
    }

    if(setsockopt(s, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) < 0){
        perror("Error on setting recv buffer size");
        return -1;
    }

    if(setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &loopback, sizeof(loopback)) < 0){
        perror("Error on loopback setting");
        return -1;
    }

    if(setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hops, sizeof(hops)) < 0){
        perror("Error on hops setting");
        return -1;
    }

    return s;
}

void printPacket(unsigned char *p, int size){
    for (size_t i = 0; i < size; i++)
        printf("[%02x]", p[i]);
    printf("\n");
}

online *newnode(int id, Alertpacket *alert){
    online *temp = malloc(sizeof(online));
    temp->end = alert->timestamp + alert->ttl;
    temp->alertID = id;
    temp->priority = priorityPacket[id];
    temp->coordX = alert->coordX;
    temp->coordY = alert->coordY;
    memcpy(temp->info, alert->info, 173);
    temp->next = NULL;
    return temp;
}

int saveAlert(int id, unsigned char *packet, Alertpacket *alert){
    /* guardar pacote */
    if (packetStack[id] == NULL){
        packetStack[id] = malloc(PACKSIZ);
        memcpy(packetStack[id], packet, PACKSIZ);
        online *temp = newnode(id, alert);
        insert(temp);
    }
    if (id == lastPacket + 1)
        lastPacket = id;
    return 0;
}

void setHelloPacket(Hellopacket *p){
    char buffer[16] = {0};
    memset(p, 0, HELLSIZ);
    p->id = 5;
    p->alertID = lastPacket;
    p->priorityAlert = lastPriority;
    p->hash = hashID;
    int f = open(sessionfile, O_RDONLY);
    read(f, buffer, 15);
    sscanf(buffer, "%hd.0 %hd.0", &p->coordX, &p->coordY);
    close(f);
}

void setUpdatePacket(Alertpacket *p, int packet){
    memset(p, 0, UPDTSIZ);
    p->id = 6;
    p->alertID = packet;
}

int checkPriority(Hellopacket *p){
    int t = p->priorityAlert;
    while (t != lastPriority){
        if (priorityPacket[++t])
            return t;
    }
    return 0;
}

RSA *loadPrivateKey(char *id, char *pwd){
    RSA *sk = RSA_new();
    int p[2] = {0};         pipe(p);
    if (!fork()) {
        dup2(p[1], 1); close(p[0]);
        if (execl(".././keymgmt", ".././keymgmt", "-p", id, pwd, NULL) < 0) {
            perror("Erro-..");
            exit(-1);
        }
    }
    close(p[1]);            wait(NULL);
    char key[1680] = {0};   read(p[0], key, 1680);

    BIO *keybio = BIO_new_mem_buf(key, -1);
    if (keybio != NULL){
        sk = PEM_read_bio_RSAPrivateKey(keybio, &sk, NULL, NULL);
        return sk;
    }else{
        perror("Erro Private key loading");
        return NULL;
    }
}

RSA *loadPublicKey(char *id){
    RSA *pk = RSA_new();

    char path[32] = {0};
    sprintf(path, "../pub/%s.pem", id);

    FILE *f = fopen(path, "rb");
    if (f != NULL){
        pk = PEM_read_RSA_PUBKEY(f, &pk, NULL, NULL);
        return pk;
    }else{
        perror("Error Public key loading");
        return NULL;
    }
}

int encrypt(unsigned char *pt, unsigned char *ct){
    if (RSA_public_encrypt(ALRTSIZ, pt, ct, pk, RSA_PKCS1_PADDING) < 0)
        return 0;
    return 1;
}

int decrypt(unsigned char *pt, unsigned char *ct){
    if (RSA_private_decrypt(PACKSIZ, ct, pt, sk, RSA_PKCS1_PADDING) < 0)
        return 0;
    return 1;
}

int check(int s){
    if (s == 0){
        clock_gettime(CLOCK_REALTIME, &ts1);
        return 0;
    }
    struct timespec ts2;
    clock_gettime(CLOCK_REALTIME, &ts2);
    if (ts2.tv_nsec < ts1.tv_nsec) {
        ts2.tv_nsec += 1000000000;
        ts2.tv_nsec--;
    }
    int t = (ts2.tv_nsec - ts1.tv_nsec)/1000000;
    if (t >= s)
        return 500;
    return (s-t);
}

void regist(unsigned char *buffer, unsigned char *cPacket, Registpacket *packet, char *registration, struct pollfd sock[2], struct sockaddr_in6 addr){
    memset(packet, 0, ALRTSIZ);

    packet->id = 3;

    getIfIpaddr("eth0", AF_INET6, packet->ip);

    memcpy(packet->registration, registration, 6);

    char config[128] = {0};
    int f = open(registration, O_RDONLY);
    if (f < 0) {
        perror("ERROR: Car config file.");
        exit(-1);
    }
    read(f, config, sizeof(config));
    char *token = strtok(config, "\n");

    int d, m, y;
    sscanf(token,"%d/%d/%d", &d, &m, &y);
    packet->date[0] = d;     packet->date[1] = m;
    packet->date[2] = y>>8;  packet->date[3] = y;

    packet->hash = hashID = hash((unsigned char*)registration);

    token = strtok(NULL, "\n");
    strcat((char *)packet->modelNowner, token);
    strcat((char *)packet->modelNowner, "#");
    token = strtok(NULL, "\n");
    strcat((char *)packet->modelNowner, token);
    close(f);

    encrypt((unsigned char *)packet, cPacket);

    Servicepacket *ack = malloc(PACKSIZ);
    while (1){
        poll(sock, 2, 500);
        if (sock[0].revents & POLLIN){
            recv(sock[0].fd, buffer, PACKSIZ, 0);
            if (decrypt((unsigned char *)ack, buffer)){
                if (ack->id == 0x08 && ack->dest == hashID){
                    free(ack);
                    return;
                }
            }
        }else{
            sendto(sock[0].fd, cPacket, PACKSIZ, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr_in6));
            sock[1].revents = 0;
        }
    }
}

int main(int argc, char *argv[]){
    system("export TERM=xterm");
    clearTerm();
    signal(SIGALRM, setAlarm);
    signal(SIGCHLD, sigchld_handler);

    sessionfile = argv[1];

    sk = loadPrivateKey("car", "1234");
    pk = loadPublicKey("car");

    unsigned char *buffer  = malloc(PACKSIZ);
    unsigned char *cPacket = malloc(PACKSIZ);
    Alertpacket   *packet  = malloc(ALRTSIZ); //ALRTSIZ

    char *multicastaddr = "FF02::1";
    int timeout = 1500;

    // MULTICAST ADDRESS'S
    struct sockaddr_in6 addr[2] = {0};
    addr[0].sin6_family = AF_INET6;     addr[0].sin6_port = htons(7777);
    inet_pton(AF_INET6, multicastaddr, &addr[0].sin6_addr);

    addr[1].sin6_family = AF_INET6;     addr[1].sin6_port = htons(6666);
    inet_pton(AF_INET6, multicastaddr, &addr[1].sin6_addr);

    struct pollfd sock[2] = {0};
    sock[0].fd = setMulticastSocket(AF_INET6, 7777, multicastaddr, "eth0", 0, 1);
    sock[0].events = POLLIN;

    sock[1].fd = setMulticastSocket(AF_INET6, 6666, multicastaddr, "eth1", 0, 1);
    sock[1].events = POLLIN;

    regist(buffer, cPacket, (Registpacket *)packet, argv[2], sock, addr[0]);

    int  alert = 0, prio = 0;
    while (1){
        check(0);
        poll(sock, 2, timeout);
        timeout = check(timeout);

        for (int i = 0; i < 2; i++){
            if (sock[i].revents & POLLIN){
                recv(sock[i].fd, buffer, PACKSIZ, 0);
                if (decrypt((unsigned char *)packet, buffer)){
                    alert = packet->alertID;
                    if (((alert>>8) & 0x80) == 0x80){
                        alert -= 0x80<<8;
                        priorityPacket[alert] = 1;
                        lastPriority = alert;
                    }
                    switch (packet->id){
                        case 0x04:
                            /* Alerta */
                            if (alert > lastPacket){
                                saveAlert(alert, buffer, packet);
                                setHelloPacket((Hellopacket *)packet);
                                encrypt((unsigned char *)packet, cPacket);
                                sendto(sock[i].fd, cPacket, PACKSIZ, 0, (struct sockaddr*)&addr[i], sizeof(struct sockaddr_in6));
                            }
                            break;
                        case 0x05:
                            /* Ola */
                            if (alert != lastPacket) {
                                if (alert < lastPacket){
                                    if ((prio = checkPriority((Hellopacket *)packet))){
                                        sendto(sock[i].fd, packetStack[prio], PACKSIZ, 0, (struct sockaddr*)&addr[i], sizeof(struct sockaddr_in6));
                                    }else{
                                        if (packetStack[alert + 1] == NULL){
                                            setUpdatePacket((Alertpacket *)packet, alert+1);
                                            encrypt((unsigned char *)packet, cPacket);
                                            sendto(sock[i].fd, cPacket, PACKSIZ, 0, (struct sockaddr*)&addr[i], sizeof(struct sockaddr_in6));
                                        }else{
                                            sendto(sock[i].fd, packetStack[alert + 1], PACKSIZ, 0, (struct sockaddr*)&addr[i], sizeof(struct sockaddr_in6));
                                        }
                                    }
                                }else{
                                    setHelloPacket((Hellopacket *)packet);
                                    encrypt((unsigned char *)packet, cPacket);
                                    sendto(sock[i].fd, cPacket, PACKSIZ, 0, (struct sockaddr*)&addr[i], sizeof(struct sockaddr_in6));
                                }
                            }
                            break;
                        case 0x06:
                            /* Alerta morto */
                            if (alert == lastPacket+1){
                                lastPacket++;
                                setHelloPacket((Hellopacket *)packet);
                                encrypt((unsigned char *)packet, cPacket);
                                sendto(sock[i].fd, cPacket, PACKSIZ, 0, (struct sockaddr*)&addr[i], sizeof(struct sockaddr_in6));
                            }
                            break;
                        case 0x07:
                            if (i == 0){
                                Servicepacket *s = (Servicepacket*) packet;
                                if (s->dest == hashID){
                                    if (serv[0] == NULL)  serv[0] = malloc(ALRTSIZ);
                                    memcpy(serv[0], s, ALRTSIZ);
                                    if (!fork()){
                                        printAlert(onlineStack);
                                        exit(0);
                                    }
                                }
                            }
                            break;
                        case 0x08:
                            if (i == 0){
                                Servicepacket *s = (Servicepacket*) packet;
                                if (s->servID) {
                                    if (s->dest == hashID){
                                        if (serv[1] == NULL)  serv[1] = malloc(ALRTSIZ);
                                        memcpy(serv[1], s, ALRTSIZ);
                                        if (!fork()){
                                            printAlert(onlineStack);
                                            exit(0);
                                        }
                                    }
                                }
                            }
                            break;
                    }
                }
                sock[i].revents = 0;
                break;
            }else{
                setHelloPacket((Hellopacket *)packet);
                encrypt((unsigned char *)packet, cPacket);
                sendto(sock[i].fd, cPacket, PACKSIZ, 0, (struct sockaddr*)&addr[i], sizeof(struct sockaddr_in6));
            }
        }
    }
}
