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

linkLayer* dataLink;

/**
 * Estabelecimento da ligação entre Transmissor e Recetor
 * @param port identifica a porta de série
 * @param status TRANSMITTER / RECEIVER
 * @return identificador da ligacao de dados ou -1 em caso de erro
*/
int llopen(int port, Status status);


/**
 * Estabelecimento da ligação: Receção da trama SET pelo Recetor e envio da trama UA
 * @param fd descritor da porta de série
 * @return 0 em caso de sucesso e -1 em caso de falha
*/
int openReceiver(int fd);


/**
 * Estabelecimento da ligação: Envio da trama SET pelo Transmissor (com timeout e número máximo de retransmissões) e receção da trama UA
 * @param fd descritor da porta de série
 * @return 0 em caso de sucesso e -1 em caso de falha
*/
int openTransmitter(int fd);


/** 
 * Inicializa a struct dataLink com os dados da ligação e da porta de série
 * @param port nome da porta de série
 * @return 0 em caso de sucesso e -1 em caso de falha
*/
int initDataLink(int port);


/**
 * Leitura da informação da porta de série
 * @param fd descritor da porta de série
 * @param buffer para armazenar os dados lidos da porta de série
 * @return comprimento do array(número de carateres lidos) ou -1 em caso de falha
*/
int llread(int fd, unsigned char* buffer);


/**
 *  Maquina de estados para receber trama de supervisão ou não numerada(SET, UA, RR, REJ, DISC)
 * @param fd descritor da porta de série
 * @param control campo de controlo da trama
 * @return 0 em caso de sucesso e -1 em caso de falha
*/
int receiveSupervisionFrame(int fd, Control control);


/**
 *  Envio de trama de supervisão/não numerada(SET, UA, RR, REJ, DISC)
 * @param fd descritor da porta de série
 * @param control campo de controlo da trama
 * @return 0 em caso de sucesso e -1 em caso de falha
*/
int sendSupervisionFrame(int fd, Control control);


/**
 * Receção da trama de informação(I)
 * @param fd descritor da porta de série
 * @return número de bytes de dados se não há erros no cabeçalho (BCC1) nem no campo de dados(BCC2) e -1 no caso de erros no campo de dados(BCC2)
*/
int receiveInfoFrame(int fd);


/**
 * Validação do BCC2 
 * @param dataField campo de dados(bcc2 inclusive)
 * @param length 
*/
int validBcc2(unsigned char * dataField, int length);

/**
 * Verifica se o campo de controlo correponde ao de uma trama de informação
 * @param byte o octeto a verificar
 * @return 0(TRUE) no caso de se tratar de uma trama de informação e 1(FALSE) caso contrário
*/
int isInfoSequenceNumber(unsigned char byte);

/**
 * Verifica se o número de série da trama recebida é o esperado
 * @return 0(TRUE) se o número de série é o esperado e 1(FALSE) caso contrário
*/
int isExpectedSequenceNumber();


/**
 * Determina resposta a enviar ao emissor da trama, consoante o número de série e a validade do campo de dados
 * @param validDataField indica se o campo de dados recebido tem erros
 * @param expectedSequenceNumber indica se o número de série recebido é o esperado
 * @return 0(TRUE) se o número de série é o esperado e 1(FALSE) caso contrário
*/
Control buildAck(int validDataField, int expectedSequenceNumber);

#endif
