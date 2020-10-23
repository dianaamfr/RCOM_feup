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


int sendFile(char *port) {

    app.st = TRANSMITTER;

    FILE *fp;
    char fileName[MAX_FILE];

    printf("Enter your file path: ");
    fgets(fileName, MAX_FILE, stdin);
    fileName[strlen(fileName) - 1] = '\0';

    if ((fp = fopen(fileName, "rt")) == NULL){
        perror("Error opening file");
        return -1;
    }

    // Estabelecer ligação entre recetor e emissor
    if ((app.fd = llopen(port, app.st)) <= 0){
        return -1;
    }

    unsigned char packet[PACKET_SIZE];
    int fileSize = sizeFile(fp); // Tamanho do ficheiro
    
    // Constrói pacote de controlo indicando inicio da transmissão
    int packetLength = controlPacket(packet, CTRL_PACKET_START, fileSize, fileName);

    // Envio do pacote inicial para o recetor através da porta de série
    if (llwrite(app.fd, packet, packetLength) < 0){ 
        fclose(fp);        
        return -1;
    }

    // Transmissão dos Pacotes Dados
    unsigned char data[DATA_SIZE];
    int length_read;
    int seqNumber = 0;

    while (TRUE){
        // Lê parte da informação do ficheiro
        length_read = fread(data, sizeof(unsigned char), DATA_SIZE, fp); 
        
        // Tamanho lido inferior ao tamanho de um bloco de dados 
        if (length_read != DATA_SIZE){
            // Fim do ficheiro
            if (feof(fp)){

                // Constrói pacote de dados
                packetLength = dataPacket(packet, seqNumber, data, length_read);
                seqNumber = (seqNumber + 1) % 256; // NS varia na range [0-255] 

                // Envia pacote de dados
                if (llwrite(app.fd, packet, packetLength) < 0){
                    fclose(fp);
                    return -1;
                }
                break; // Terminou a leitura
            }
            else{
                perror("Error reading from file\n");
                return -1;
            }
        }

        // Tamanho lido igual ao tamanho de um bloco de dados
        packetLength = dataPacket(packet, seqNumber, data, length_read);
        seqNumber = (seqNumber + 1) % 256; // NS varia na range [0-255] 
        if (llwrite(app.fd, packet, packetLength) < 0){
            fclose(fp);            
            return -1;
        }
    }

    // Envia pacote de controlo indicando o fim da transmissão
    packetLength = controlPacket(packet,CTRL_PACKET_END, fileSize, fileName);
    if (llwrite(app.fd, packet, packetLength) < 0){ 
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

    if ((app.fd = llopen(port, app.st)) < 0){
        perror("Error llopen");
        return -1;
    }

    unsigned char packet[PACKET_SIZE]; // Pacote
    unsigned char data[DATA_SIZE]; // Dados
    int packetLength, fileSize;
    char fileName[MAX_FILE];


    if ((packetLength = llread(app.fd, packet)) < 0){
        perror("Error llread");
        return -1;
    }

    // Pacote de Controlo
    if (packet[0] != CTRL_PACKET_START){
        perror("Should have received Start Control Packet");
        return -1;
    }
    
    if (readControlPacket(packet, &fileSize, fileName) < 0){
        perror("Error Reading Start Control Packet");
        return -1;
    }

    // Cria o ficheiro
    FILE *fp;
    if ((fp = fopen(fileName, "w")) == NULL){
        perror("Error opening file - fopen");
        return -1;
    }

    int sequenceNumberConfirm = 0;
    int receivedSeqNumber, dataLength;

    // Pacote de Dados
    while (TRUE){
        memset(packet, 0, PACKET_SIZE);

        if ((packetLength = llread(app.fd, packet)) < 0){
            perror("Error reading data packet from port with llread");
            return -1;
        }
        
        // Nothing was read
        if(packetLength == 0)
            continue;

        // Processar bloco de dados
        if (packet[0] == CTRL_PACKET_DATA){
            
            if (readDataPacket(packet, data, &receivedSeqNumber) < 0){
                perror("Error reading data packet");
                return -1;
            }

            if (sequenceNumberConfirm != receivedSeqNumber){ // Número de sequencia não coincide
                printf("Error sequence number \n");
                return -1;
            }

            sequenceNumberConfirm = (sequenceNumberConfirm + 1) % 256; //  NS varia na range [0-255] 

            dataLength = packetLength - PACKET_HEADER;

            if (fwrite(data, sizeof(unsigned char), dataLength, fp) != dataLength){ //Escreve no ficheiro
                perror("Error writing in file - fwrite");
                return -1;
            }
        }

        // Terminar receção de dados => chegou pacote que indica fim de transferência
        else if (packet[0] == CTRL_PACKET_END)
            break;
    }


    if (sizeFile(fp) != fileSize){
        printf("END fileSize does not match fileSize");
        return -1;
    }

    int fileSizeEndPart;
    char fileNameEndPart[255];

    if (readControlPacket(packet, &fileSizeEndPart, fileNameEndPart) < 0){
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

    // L2 | L1 => número de octetos do campo de dados
    // K = 256 * L2 + L1
    int packetlength1 = dataLength % 256;
    int packetlength2 = dataLength / 256;

    packet[0] = CTRL_PACKET_DATA;
    packet[1] = (unsigned char)seqNumber; 
    packet[2] = (unsigned char)packetlength2;
    packet[3] = (unsigned char)packetlength1;

    for (int i = 0; i < dataLength; i++){
        packet[i + PACKET_HEADER] = bufferData[i];
    }

    return dataLength + PACKET_HEADER;
}


int controlPacket(unsigned char *packet,unsigned char controlByte,  int fileSize, char *fileName){
    // C | T1 | L1 | V1 | T2 | L2 | V2
    // Tamanho do pacote de controlo = 3 octetos iniciais(C,T1,L1) + octetos ocupados por V1 + 2 octetos para T2 e L2 + Octetos ocupados pelo nome do ficheiro
    int packetSize = 0;

    packet[0] = controlByte; // C
    

    // Relativos ao tamanho do ficheiro: T1 | L1 | V1 
    int l1 = buildV1(packet, fileSize);
    packet[1] = FILESIZE; // T1
    packet[2] = (unsigned char)l1; // L1

    packetSize += 3 + l1; // 3 octetos iniciais(C,T1,L1) + octetos ocupados por V1


    // Relativos ao nome do ficheiro: T2 | L2 | V2
    packet[3 + l1] = FILENAME; // T2
    int T2 = 4 + l1; // T2 =Z Filename Len
    int L2 = 5 + l1; // L2 => Filename
    int fileNameSize = (strlen(fileName) + 1);
    
    // Adiciona comprimento do nome do ficheiro
    packet[T2] = (unsigned char)fileNameSize; // L2

    for (unsigned int i = 0; i < fileNameSize; i++){
        packet[L2 + i] = fileName[i]; // V2
    }

    packetSize += 2 + fileNameSize; // 2 octetos para T2 e L2 + Octetos ocupados pelo nome do ficheiro

    return packetSize;
}


int readDataPacket(unsigned char *packet, unsigned char *data, int *seqNumber){

    if (packet[0] != CTRL_PACKET_DATA){
        return -1;
    }

    *seqNumber = (int)packet[1];

    int datalength = 256 * (int)packet[2] + (int)packet[3]; // K = 256 * L1 * L2

    for (int i = 0; i < datalength; i++){
        data[i] = packet[i + PACKET_HEADER];
    }

    return 0;
}


int readControlPacket(unsigned char *packet, int *fileSize, char *fileName){
   
    int length1,length2;

    if (packet[0] != CTRL_PACKET_START && packet[0] != CTRL_PACKET_END){
        perror("Not a Valid Control Packet");
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


int buildV1(unsigned char *packet, int fileSize){
    int fileSizeLen = 0; // Numero de bytes ocupados pelo tamanho do ficheiro no packet a enviar
    int sizeToProcess = fileSize;
    int rest;

    // Conversão do tamanho do ficheiro para octetos
    while (sizeToProcess > 0){  

        rest = sizeToProcess % 256; 
    
        fileSizeLen++;
        // Deslocamento dos bytes para a direita para inserção do novo byte
        // fileSize = rest1 + rest2 * 256 + rest3 * 256^2 + ...
        for (unsigned int i = PACKET_L1 + fileSizeLen; i > PACKET_V1; i--){ 
            packet[i] = packet[i - 1];
        }

        packet[PACKET_V1] = (unsigned char)rest; // V1
        sizeToProcess = sizeToProcess / 256; 
    }

    return fileSizeLen;
}
