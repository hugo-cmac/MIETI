#ifndef RECEBER_FICHEIRO_H
#define RECEBER_FICHEIRO_H

    #include "camada_de_ligacao.h"
    extern int web;
    

    void file(char op , char *filename){
        char fn[BUFFERSIZE]; memset(fn, '\0', BUFFERSIZE);
        sprintf(fn, "-%c%s",op ,filename);
        write(web, fn, strlen(fn));
        close(web);
        msleep(1);
        web = open("toWeb", O_WRONLY);
    }

    int iniciatefilebuild(filenS *packet,int length){
        f = malloc(sizeof(fileS));
        if((f->fd = open((char *)packet->FileName, O_CREAT | O_WRONLY | O_TRUNC , 0666)) < 0){
            perror("Erro na abertura do ficheiro.");
            return -1;
        }
        strncpy((char *)f->FileName, (char *)packet->FileName, strlen((char *)packet->FileName));
        f->size = getHead(sizeof(packet->OpcodeAndSize), packet->OpcodeAndSize);
        s = f->size;
        return 0;
    }

    int filebuild(dataS *packet, int length, unsigned int seq){
        write(f->fd, packet->data, length);
        f->size = f->size - length;
        return 0;
    }

    int findError(int serie, unsigned char *packet, int length, unsigned char op, unsigned int seq){
        unsigned char crc = packet[length-1];
        packet[length-1]=0;

        if (crc == gencrc(packet,length) && op == (packet[0] & 0xf0)) {
            if (OPCODEDP == (packet[0] & 0xf0)){
                if (seq != getHead(3,((dataS*) packet)->OpcodeAndPacket))    return -1;
            }
            return 0;
        }else if (crc == gencrc(packet,length) && OPCODEDEL == (packet[0] & 0xf0)){
            file( 'd', (char *)((filenS *) packet)-> FileName);
            return 4;
        }
        return -1;
    }

    int receive(int serial, unsigned char *packet, int length, unsigned int seq, unsigned char op){
        unsigned char confirmpacket[ACKSIZE]; memset(confirmpacket,0,ACKSIZE);
        int confirm = -1, trys=0;

        while ( confirm != 0) {
            if (serialRead(serial, packet, length)!=0) {
                confirm = findError(serial, packet, length, op, seq);
                printf("Confirmation: %d\n", confirm);
                if (confirm == 0){     headMaker(ACKSIZE,confirmpacket,OPCODEACK,seq);
                }else if (confirm == 4){     
                    headMaker(ACKSIZE,confirmpacket,OPCODEACK,0); 
                    serialWrite(serial,confirmpacket,ACKSIZE);
                    return 4;
                }else{     headMaker(ACKSIZE,confirmpacket,OPCODENACK,seq);}
                serialWrite(serial,confirmpacket,ACKSIZE);
            }else if(op == OPCODEFNP){
                msleep(300);
            }else{      trys++;     if (trys == 30){    return -1;}}
        }
        return confirm;
    }

    int receiveFile(int serial){
        unsigned char *packet;
        unsigned int seq = 0;
        int r;

        packet = malloc( sizeof(filenS));    memset(packet, '\0', sizeof(filenS));

        if ((r = receive(serial, packet, sizeof(filenS), 1048575, OPCODEFNP)) == 0){
            iniciatefilebuild( (filenS *)packet, sizeof(filenS));
            free(packet);    packet = malloc(sizeof(dataS));

            while (f->size != 0) {
                printf("Sequence: %d\n", seq);
                seq++;
                if (f->size < DATASIZE) {
                    if ((r = receive(serial, packet, f->size + 4, seq, OPCODEDP)) == -1)    break;
                    filebuild((dataS *)packet, f->size, seq);
                }else{
                    if ((r = receive(serial, packet, sizeof(dataS), seq, OPCODEDP)) == -1)    break;
                    filebuild((dataS *)packet,DATASIZE,seq);
                }
            }
        }
        if (r == -1){
            unlink(f->FileName);
        }
        if (r == 4){
            serialClose(serial);
            free(packet);
        }else{
            closefds(serial);
            free(packet);
        }
        return 0;
    }

#endif