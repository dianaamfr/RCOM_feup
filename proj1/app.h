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
#include "macros.h"


typedef struct appLayer {
    int fd;     /*Descritor correspondente à porta série*/
    Status st;  /*TRANSMITTER, RECEIVER*/
} appLayer;

appLayer app;

/**
 * Envia um ficheiro usando a porta série
 * @param port Nome da porta série
 * @return 
 */
int sendFile(char *port);

/**
 * Recebe um ficheiro enviado pela porta série
 * @param port Nome da porta série
 * @return
 */
int receiveFile(char *port);

/**
 * Constrói o campo de controlo, recebendo um byte de controlo e a informação do ficheiro. 
 * Retorna a informação no packet.
 * @param packet Buffer que vai conter a informação final de packet
 * @param controlByte 
 * @param fileSize 
 * @param fileName 
 * @return Tamanho do packet
 */
int controlPacket(unsigned char *packet,unsigned char control,  int fileSize, char *fileName);

/**
 * Constrói o campo de dados, recebendo um número de sequência e um buffer com os bytes
 * a serem enviados. Retorna a informação no packet.
 * @param packet Buffer que vai conter a informação final de packet
 * @param sequenceNumber Número de sequência do pacote
 * @param dataBuffer Buffer com os dados
 * @param dataLength Tamanho dos dados no buffer
 * @return Tamanho do packet
 */
int dataPacket(unsigned char *packet, int sequenceNumber, unsigned char *dataBuffer, int dataLength);

int parseControlPacket(unsigned char *packet, int *fileSize, char *fileName);

/**
 * Interpreta o pacote de dados no receiver
 * @param packet Buffer com o pacote de dados
 * @param data Buffer que vai conter a informação final 
 * @param sequenceNumber Número de sequência do pacote, devolvido pela função
 * @return
 */
int parseDataPacket(unsigned char *packet, unsigned char *data, int *sequenceNumber);

