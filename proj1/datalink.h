#ifndef DATALINK_HEADER
#define DATALINK_HEADER

#include "macros.h"

typedef struct linkLayer {
    char port[20];                  /*Dispositivo /dev/ttySx, x = 0, 1*/
    int baudRate;                   /*Velocidade de transmissão*/
    unsigned int sequenceNumber;    /*Número de sequência da trama: 0, 1*/
    unsigned int timeout;           /*Valor do temporizador: 1 s*/
    unsigned int numTransmissions;  /*Número de tentativas em caso de falha*/
    unsigned char frame[MAX_INFO_FRAME];     /*Trama*/
} linkLayer;

linkLayer dataLink;

/**
 * Estabelecimento da ligação entre transmissor e recetor
 * @param port identifica a porta de serie
 * @param role TRANSMITTER / RECEIVER
 * @return identificador da ligacao de dados ou valor negativo em caso de erro
*/
int llopen(int port, Status status);

int openReceiver(int fd);

int openTransmitter(int fd);

/**
 *  Maquina de estados para receber uma trama de controlo 
 * 
 * @param fd descritor da porta de serie
*/
int receiveSupervisionFrame(int fd, Control C);

int sendSupervisionFrame(int fd, Control C);

int receiveInfoFrame(int fd, Control control);

int validBcc2(unsigned char * dataField, int length);

#endif
