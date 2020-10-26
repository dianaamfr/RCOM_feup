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


int sendFile(char* port) {

    app.st = TRANSMITTER;

    FILE *fp;
    fileData fileDt;

    printf("Enter your file path: ");
    fgets(fileDt.fileName, MAX_FILE, stdin);
    fileDt.fileName[strlen(fileDt.fileName) - 1] = '\0';

    if ((fp = fopen(fileDt.fileName, "rt")) == NULL){
        perror("Error opening file");
        return -1;
    }

    // Estabelecer ligação entre recetor e emissor
    if ((app.fd = llopen(port, app.st)) <= 0){
        perror("Error in llopen");
        fclose(fp);      
        return -1;
    }

    unsigned char packet[PACKET_SIZE];
    int packetLength;

    // Tamanho do ficheiro
    fileDt.fileSize = sizeFile(fp);

    // Constrói pacote de controlo indicando inicio da transmissão
    packetLength = controlPacket(packet, CTRL_PACKET_START, &fileDt);

    // Envio do pacote inicial para o recetor através da porta de série
    if (llwrite(app.fd, packet, packetLength) < 0){ 
        perror("Error in llwrite sending start packet");
        fclose(fp);        
        return -1;
    }

    // Transmissão dos Pacotes Dados
    packetData data;
    int seqNumber = 0;

    while (TRUE){

        // Tamanho lido inferior ao tamanho de um bloco de dados e não é fim de ficheiro
        if ((data.len = fread(data.buf, sizeof(unsigned char), DATA_SIZE, fp)) != DATA_SIZE && !feof(fp)){
            if(fclose(fp) < 0){
                perror("Error closing fd");            
                return -1;
            }
            perror("Error reading from file");
            return -1;
        }

        printf("\nApp: ns = %d\n", seqNumber);
        data.ns = seqNumber;
        packetLength = dataPacket(packet, &data);
        seqNumber = (seqNumber + 1) % 256; // NS varia na range [0-255] 
        
        if (llwrite(app.fd, packet, packetLength) < 0){
            if(fclose(fp) < 0){
                perror("Error closing fd");            
                return -1;
            }
            perror("Error llwrite - sending data packet");            
            return -1;
        }

        // Fim do ficheiro
        if(feof(fp)){
            if(fclose(fp) < 0){
                perror("Error closing fd");            
                return -1;
            }
            break;
        } 
            
    }

    printf("\nApp: ns = %d\n", seqNumber);
    // Envia pacote de controlo indicando o fim da transmissão
    packetLength = controlPacket(packet,CTRL_PACKET_END, &fileDt);
    if (llwrite(app.fd, packet, packetLength) < 0){ 
        perror("Error llwrite - sending end packet\n");           
        return -1;
    }

    if (llclose(app.fd, app.st) < 0){
        perror("Error llclose\n");     
        return -1;
    }

    return 0;
}


int receiveFile(char* port){

    app.st = RECEIVER;

    if ((app.fd = llopen(port, app.st)) < 0){
        perror("Error llopen");
        return -1;
    }

    unsigned char packet[PACKET_SIZE]; // Pacote
    int packetLength = 0;

    while(packetLength == 0){
        if ((packetLength = llread(app.fd, packet)) < 0){
            perror("Error llread");
            return -1;
        }
    }

    // Pacote de Controlo
    if (packet[0] != CTRL_PACKET_START){
        perror("Should have received Start Control Packet");
        return -1;
    }   

    fileData fileStartData;
    if (readControlPacket(packet, &fileStartData) < 0){
        perror("Error Reading Start Control Packet");
        return -1;
    }

    // Cria o ficheiro
    FILE *fp;
    if ((fp = fopen(fileStartData.fileName, "w")) == NULL){
        perror("Error opening file - fopen");
        return -1;
    }

    int sequenceNumberConfirm = 0;
    packetData data; // Dados

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
            
            if (readDataPacket(packet, &data) < 0){
                perror("Error reading data packet");
                return -1;
            }

            if (sequenceNumberConfirm != data.ns){ // Número de sequencia não coincide
                perror("Error sequence number");
                return -1;
            }
            
            printf("\nApp: ns = %d\n", sequenceNumberConfirm);
            sequenceNumberConfirm = (sequenceNumberConfirm + 1) % 256; //  NS varia na range [0-255] 

            if (fwrite(data.buf, sizeof(unsigned char), data.len, fp) != data.len){ //Escreve no ficheiro
                perror("Error writing in file - fwrite");
                return -1;
            }
        }

        // Terminar receção de dados => chegou pacote que indica fim de transferência
        else if (packet[0] == CTRL_PACKET_END){
            printf("\nApp: ns = %d\n", sequenceNumberConfirm);
            break;
        }
    }


    if (sizeFile(fp) != fileStartData.fileSize){
        perror("FileSize expected does not match actual fileSize");
        return -1;
    }

    fileData fileEndData;
    if (readControlPacket(packet, &fileEndData) < 0){
        perror("Error reading End packet");
        return -1;
    }
    if((fileStartData.fileSize != fileEndData.fileSize )){
        perror("Error, final file size does not match initial");
        return -1;
    }

    if(strcmp(fileStartData.fileName, fileEndData.fileName) != 0){
        perror("Error, final file name doesn't match initial");
        return -1;
    }

    if (llclose(app.fd, app.st) < 0){
        perror("Error llclose");
        return -1;
    }

    return 0;
}


int dataPacket(unsigned char* packet, packetData* data){

    packet[0] = CTRL_PACKET_DATA; // C
    packet[1] = (unsigned char)(data->ns); // Ns
    // L2 | L1 => número de octetos do campo de dados
    // K = 256 * L2 + L1
    packet[2] = (unsigned char)(data->len / 256); // L2
    packet[3] = (unsigned char)(data->len % 256); // L1

    for (int i = 0; i < data->len; i++)
        packet[i + PACKET_HEADER] = data->buf[i];

    return data->len + PACKET_HEADER;
}


int controlPacket(unsigned char* packet, PacketControl control,  fileData* fileDt){
    // C | T1 | L1 | V1 | T2 | L2 | V2
    // Tamanho do pacote de controlo = 3 octetos iniciais(C,T1,L1) + octetos ocupados por V1 + 2 octetos para T2 e L2 + Octetos ocupados pelo nome do ficheiro(V2)
    int packetSize = 0;

    packet[0] = control; // C
    

    // Relativos ao tamanho do ficheiro: T1 | L1 | V1 
    int l1 = buildV1(packet, fileDt->fileSize);
    packet[1] = FILESIZE; // T1
    packet[2] = (unsigned char)l1; // L1

    packetSize += 3 + l1; // 3 octetos iniciais(C,T1,L1) + octetos ocupados por V1


    // Relativos ao nome do ficheiro: T2 | L2 | V2
    int T2 = 3 + l1;
    packet[T2] = FILENAME; // T2
    int L2 = T2 + 1; // L2 => Filename Len
    int V2 = L2 + 1; // V2 => Filename
    int fileNameSize = (strlen(fileDt->fileName) + 1);
    
    // Adiciona comprimento do nome do ficheiro
    packet[L2] = (unsigned char)fileNameSize; // L2

    for (unsigned int i = 0; i < fileNameSize; i++){
        packet[V2 + i] = fileDt->fileName[i]; // V2
    }

    packetSize += 2 + fileNameSize; // 2 octetos para T2 e L2 + Octetos ocupados pelo nome do ficheiro (V2)

    return packetSize;
}


int readDataPacket(unsigned char* packet, packetData* data){

    if (packet[0] != CTRL_PACKET_DATA){
        perror("Unexpected type of Packet - not a data packet");
        return -1;
    }

    data->ns = (int)packet[1];

    data->len = 256 * (int)packet[2] + (int)packet[3]; // K = 256 * L2 + L1

    for (int i = 0; i < data->len; i++){
        data->buf[i] = packet[i + PACKET_HEADER];
    }

    return 0;
}


int readControlPacket(unsigned char* packet, fileData* fileDt){
   
    int l1,l2;

    if (packet[0] != CTRL_PACKET_START && packet[0] != CTRL_PACKET_END){
        perror("Not a Valid Control Packet");
        return -1;
    }

    // Leitura dos octetos relativos ao tamanho do ficheiro
    if (packet[1] != FILESIZE){
       perror("FileSize was not found in the right byte of the packet");
       return -1;
    }
    
    fileDt->fileSize = 0;
    l1 = (int)packet[2];
    for (int i = 0; i < l1; i++){
        fileDt->fileSize = fileDt->fileSize * 256 + (int)packet[3 + i]; // Filesize = resto1 + resto2 * 256 + resto3 * 256^2 ... (Bytes to decimal)
    }
    
    // Leitura dos octetos relativos ao nome do ficheiro
    int T2 = 3 + l1; // Onde começa a parte do nome do ficheiro
    int L2 = T2 + 1;
    int V2 = L2 + 1;

    if (packet[T2] != FILENAME){
        perror("FileName was not found in the right byte of the packet");
        return -1;
    }

    l2 = (int)packet[L2];
    for (int i = 0; i < l2; i++){
        fileDt->fileName[i] = packet[V2 + i];
    }

    return 0;
}


int buildV1(unsigned char* packet, int fileSize){
    int fileSizeLen = 0; // Vai guardar o numero de bytes ocupados pelo tamanho do ficheiro no packet a enviar
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
