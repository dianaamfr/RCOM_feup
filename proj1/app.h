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
 * @return 0 em caso de sucesso e -1 em caso de insucesso
 */
int sendFile(char* port);

/**
 * Recebe um ficheiro enviado pela porta série
 * @param port Nome da porta série
 * @return 0 em caso de sucesso e -1 em caso de insucesso
 */
int receiveFile(char* port);

/**
 * Constrói o campo de controlo, recebendo um byte de controlo e a informação do ficheiro. 
 * Retorna a informação no packet.
 * @param packet Buffer que vai conter a informação final de packet
 * @param control primeiro byte do pacote - campo de controlo
 * @param fileSize tamanho do ficheiro
 * @param fileName nome do ficheiro
 * @return Tamanho do pacote
 */
int controlPacket(unsigned char* packet,unsigned char control, int fileSize, char* fileName);

/**
 * Constrói o campo de dados, recebendo um número de sequência e um buffer com os bytes
 * a serem enviados. Retorna a informação no pacote de dados recebido nos argumentos.
 * @param packet Buffer que vai conter a informação final do pacote
 * @param seqNumber Número de sequência do pacote
 * @param data Buffer com os dados
 * @param len Tamanho dos dados no buffer
 * @return Tamanho do pacote
 */
int dataPacket(unsigned char* packet, int seqNumber, unsigned char* data, int len);

/**
 * Faz a leitura de um pacote de controlo
 * @param packet Pacote recebido
 * @param fileSize tamanho do ficheiro - a ser lido do pacote e retornado nos argumentos
 * @param fileName nome do ficheiro - a ser lido do pacote e retornado nos argumentos
 * @return 0 em caso de sucesso e -1 em caso de insucesso
 */
int readControlPacket(unsigned char* packet, int* fileSize, char* fileName);

/**
 * Interpreta o pacote de dados no receiver
 * @param packet Buffer com o pacote recebido
 * @param data Buffer que vai conter os dados do pacote 
 * @param seqNumber Número de sequência do pacote - a ser lido do pacote e retornado nos argumentos
 * @return 0 em caso de sucesso e -1 em caso de insucesso
 */
int readDataPacket(unsigned char* packet, unsigned char* data, int* seqNumber);

/**
 * Constrói o campo V1 do pacote convertendo o tamanho em número inteiro para octetos
 * @param packet o pacote de dados onde deve ser guardado o tamanho do ficheiro
 * @param fileSize tamanho do ficheiro
 * @return número de octetos ocupados pelo campo V1 - L1
 */
int buildV1(unsigned char* packet, int fileSize);
