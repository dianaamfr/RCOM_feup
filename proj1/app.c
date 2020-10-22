#include <sys/times.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include "datalink.h"
#include <signal.h>
#include <string.h>
#include "alarm.h"
#include "utils.h"
#include "app.h"


int sendFile(char *port, char *fileName) {
    app.st=TRANSMITTER;

    FILE *fp = fopen(fileName, "r");
    if (fp == NULL){
        printf("No file to send\n");
        return -1;
    }

    if ((app.fd = llopen(port, app.st)) <= 0){ //Envia a trama de Supervisão SET e recebe a trama UA
        return -1;
    }

    unsigned char packet[DATA_PACKET_SIZE];
    int fileSize = sizeFile(fp); //Tamanho do ficheiro
    
    //Pacote Controlo
    int packetLength = controlPacket(packet,CTRL_PACKET_START, fileSize, fileName); //Constrói pacote de controlo indicando inicio da transmissão

    if (llwrite(app.fd, packet, packetLength) < 0){ //Stuffing e envia tramas de informação para recetor
        fclose(fp);        
        return -1;
    }

    //Pacote Dados
    unsigned char data[DATA_PACKET_SIZE];
    int length_read;
    int seqNumber = 0;

    while (1){
        length_read = fread(data, sizeof(unsigned char), DATA_PACKET_SIZE, fp);  // Lê parte da informação do ficheiro
        if (length_read != DATA_PACKET_SIZE){
            if (feof(fp)){

                packetLength = dataPacket(packet, seqNumber, data, length_read); //Constrói pacote de dados
                seqNumber = (seqNumber + 1) % 256;
                if (llwrite(app.fd, packet, packetLength) < 0){
                    fclose(fp);
                    return -1;
                }
                break;
            }
            else{
                perror("Error reading from file\n");
                return -1;
            }
        }

        packetLength = dataPacket(packet, seqNumber, data, length_read);
        seqNumber = (seqNumber + 1) % 256;
        if (llwrite(app.fd, packet, packetLength) < 0){
            fclose(fp);            
            return -1;
        }
    }

    //Pacote Controlo
    packetLength = controlPacket(packet,CTRL_PACKET_END, fileSize, fileName);
    if (llwrite(app.fd, packet, packetLength) < 0){ //Envia pacote de controlo indicando o fim da transmissão
        fclose(fp);        
        return -1;
    }

    if (llclose(app.fd, app.st) < 0){
        return -1;
    }
    if (fclose(fp) != 0){
        return -1;
    }
    return 0;
}


int receiveFile(char *port){
    app.st = RECEIVER;

    if ((app.fd = llopen(port, app.st)) <= 0){
        return -1;
    }

    unsigned char packet[DATA_PACKET_SIZE]; //(pacote total)
    int packetLength;
    int fileSize;
    char fileName[255];

    unsigned char data[DATA_PACKET_SIZE]; //(pacote de dados)

    packetLength = llread(app.fd, packet);
    if (packetLength < 0){
        return -1;
    }

    // Pacote de Controlo
    if (packet[0] == CTRL_PACKET_START){
        if (parseControlPacket(packet, &fileSize, fileName) < 0){
            return -1;
        }
    }
    else{
        return -1;
    }

    FILE *fp = fopen("teste.jpg", "w"); //Cria o ficheiro
    if (fp == NULL){
        return -1;
    }

    int sequenceNumberConfirm = 0;

    // Pacote de Dados
    while (1){
        packetLength = llread(app.fd, packet);
        if (packetLength < 0){
            return -1;
        }
  
        if (packet[0] == CTRL_PACKET_DATA){
            int seqNumber;

            if (parseDataPacket(packet, data, &seqNumber) < 0){
                return -1;
            }

            if (sequenceNumberConfirm != seqNumber){ //Número de sequencia não coincide
                printf("Error sequence number \n");
                return -1;
            }

            sequenceNumberConfirm = (sequenceNumberConfirm + 1) % 256; //?

            int dataLength = packetLength - 4;

            if (fwrite(data, sizeof(unsigned char), dataLength, fp) != dataLength){ //Escreve no ficheiro
                return -1;
            }
        }
        else if (packet[0] == CTRL_PACKET_END){
            break;
        }
    }

    if (sizeFile(fp) != fileSize){
        printf("Error fileSize\n");
        return -1;
    }

    int fileSizeEndPart;
    char fileNameEndPart[255];

    if (parseControlPacket(packet, &fileSizeEndPart, fileNameEndPart) < 0){
        return -1;
    }
    if((fileSize != fileSizeEndPart) || (strcmp(fileNameEndPart, fileName) != 0)){
        printf("Erro, informação do fim do ficheiro não coincide com o início\n");
        return -1;
    }

    if (llclose(app.fd, app.st) < 0)
        return -1;

    return 0;
}


int dataPacket(unsigned char *packet, int seqNumber, unsigned char *bufferData, int dataLength){

    int packetlength1 = dataLength % 256;
    int packetlength2 = dataLength / 256;

    packet[0] = CTRL_PACKET_DATA;
    packet[1] = (unsigned char)seqNumber; 
    packet[2] = (unsigned char)packetlength2;
    packet[3] = (unsigned char)packetlength1;

    for (int i = 0; i < dataLength; i++){
        packet[i + 4] = bufferData[i];
    }

    return dataLength + 4;
}


int controlPacket(unsigned char *packet,unsigned char controlByte,  int fileSize, char *fileName){
    //CONFIRMAR

    //(cada parametro é codificado em Type(fileName/fileSize), Length, Value)

    int length = 0;
    int currentFileSize = fileSize;

    while (currentFileSize > 0){    //Separar ficheiro em bytes V1
        int length1 = currentFileSize % 256;
        int length2 = currentFileSize / 256;
        length++;
        for (unsigned int i = 2 + length; i > 3; i--){ //ShiftRight dos bytes, para meter o novo byte
            packet[i] = packet[i - 1];
        }
        packet[3] = (unsigned char)length1;
        currentFileSize = length2;
    }

    packet[0] = controlByte;
    packet[1] = FILESIZE;
    packet[2] = (unsigned char)length;
    packet[3 + length] = FILENAME;


    int fileNamePart = 5 + length; // V2

    packet[4 + length] = (unsigned char)(strlen(fileName) + 1); // Adiciona comprimento do nome do ficheiro
    for (unsigned int j = 0; j < (strlen(fileName) + 1); j++){  // '\0' 
        packet[fileNamePart + j] = fileName[j];
    }

    return 3 + length + 2 + strlen(fileName) + 1; // Comprimento total do packet
}


int parseDataPacket(unsigned char *packet, unsigned char *data, int *seqNumber){

    if (packet[0] != CTRL_PACKET_DATA){
        return -1;
    }

    *seqNumber = (int)packet[1];

    int datalength= 256 * (int)packet[2] + (int)packet[3]; //K=256*L1*L2

    for (int i = 0; i < datalength; i++){
        data[i] = packet[i + 4];
    }

    return 0;
}


int parseControlPacket(unsigned char *packet, int *fileSize, char *fileName){
    int length1,length2;

    if (packet[0] != CTRL_PACKET_START && packet[0] != CTRL_PACKET_END){
        return -1;
    }

    if (packet[1] == FILESIZE){
        *fileSize = 0;
        length1 = (int)packet[2];
        for (int i = 0; i < length1; i++){
            *fileSize = *fileSize * 256 + (int)packet[3 + i];
        }
    }
    else{
        return -1;
    }

    int fileNamePart = 5 + length1; //Onde começa a parte do nome do ficheiro

    if (packet[fileNamePart - 2] == FILENAME){
        length2 = (int)packet[fileNamePart - 1];
        for (int i = 0; i < length2; i++){
            fileName[i] = packet[fileNamePart + i];
        }
    }
    else{
        return -1;
    }

    return 0;
}

