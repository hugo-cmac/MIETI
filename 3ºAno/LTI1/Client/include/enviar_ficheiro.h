#ifndef ENVIAR_FICHEIRO_H
#define ENVIAR_FICHEIRO_H

    #include "camada_de_ligacao.h"
    int lastpacketsize = 0;
    
    int openFile(char *filename){
        f = malloc(sizeof(fileS));
        
        if((f->fd = open(filename, O_RDONLY)) < 0){
            perror("Erro na abertura do ficheiro");
            return -1;
        }
        f->size = lseek( f->fd, 0, SEEK_END);
        s = f->size;

        if (f->size > MAXSIZE) {
            perror("File size not suported\n");
            return -1;
        }
        lseek( f->fd, 0, SEEK_SET);
        return 0;
    }

    void buildfilenamepacket(filenS *packet, char *filename){
        headMaker(4, packet->OpcodeAndSize, OPCODEFNP, f->size);
        strncpy((char *)packet->FileName, filename, strlen(filename));
        strncpy((char *)f->FileName, filename, strlen(filename));
        packet->crc = gencrc((unsigned char *)packet, sizeof(filenS));
    }

    void builddatapacket(dataS *packet,unsigned int seq){
        memset(packet, 0, sizeof(dataS));

        headMaker(3, packet->OpcodeAndPacket, OPCODEDP, seq);
        if (f->size < DATASIZE) {
            read( f->fd, packet->data, f->size);
            lastpacketsize = f->size;
            packet->crc = gencrc((unsigned char *)packet, lastpacketsize+4);
            f->size = 0;
        }else{
            read( f->fd, packet->data, DATASIZE);
            f->size = f->size - DATASIZE;
            packet->crc = gencrc((unsigned char *)packet, sizeof(dataS));
        }
    }

    void adjustdatapacket(unsigned char *packet, int length){
        packet[length-1] = packet[sizeof(dataS)-1];
    }

    int confirmation(unsigned char *packet, unsigned int seq){
        unsigned char op = packet[0];
        op = op & 0xf0;

        if (op == OPCODEACK)    return 0;
        else if (seq!=1048575) {
           if (seq < getHead(3,((dataS*)packet)->OpcodeAndPacket))    return 0;
        }
        return -1;
    }

    int send(int serial, unsigned char *packet, int length, unsigned int seq){
        unsigned char confirmpacket[ACKSIZE];   memset(confirmpacket,0,ACKSIZE);
        int confirm = -1, trys = 0;
        while ( confirm != 0) {
            serialWrite(serial, packet, length);
            if (serialRead(serial,confirmpacket,ACKSIZE) != 0){ confirm = confirmation( confirmpacket, seq);
            }else{      trys++;     if (trys == 30){    return -1;}}
        }
        return 0;
    }

    void deletefile(int serial, char *filename){
        filenS *packet = malloc(sizeof(filenS));    memset(packet, '\0', sizeof(filenS));
        
        headMaker(4, packet->OpcodeAndSize, OPCODEDEL, 0);
        strncpy((char *)packet->FileName, filename, strlen(filename));
        packet->crc = gencrc((unsigned char *)packet, sizeof(filenS));
        
        send(serial, (unsigned char *)packet, sizeof(filenS), 0);
        
        free(packet);
        serialClose(serial);
    }

    int sendfile(int serial, char *filename){
        unsigned char *packet;
        unsigned int seq = 0;
        
        if(openFile(filename) == -1) return -1;

        packet = malloc(sizeof(filenS));    memset(packet, '\0', sizeof(filenS));

        buildfilenamepacket((filenS *)packet, filename);

        if (send(serial, packet, sizeof(filenS), 1048575) == 0){
            free(packet);   packet = malloc(sizeof(dataS));
            while (f->size != 0) {
                seq++;
                builddatapacket((dataS *)packet,seq);
                if (lastpacketsize!=0) {
                    adjustdatapacket(packet,lastpacketsize+4);
                    if (send(serial, packet, lastpacketsize+4, seq) == -1)     break;
                }else{
                    if (send(serial, packet, sizeof(dataS), seq) == -1)     break;
                }
            }
        }
        
        lastpacketsize = 0;
        closefds(serial);
        free(packet);
        return 0;
    }

#endif