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
struct termios oldtio;

// Estabelecimento da ligação de dados

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


// Receção e envio de tramas de supervisão

/**
 *  Maquina de estados para receber trama de supervisão ou não numerada(SET, UA, RR, REJ, DISC)
 * @param fd descritor da porta de série
 * @param period fase do protocolo de ligação de dados
 * @param status interviniente que recebe a frame
 * @return campo de controlo em caso de sucesso e -1 em caso de falha
*/
int receiveSupervisionFrame(int fd, Period period, Status status);

/**
 * Verifica se o Campo de Controlo é o esperado pelo interviniente da ligação de dados que espera a trama dependendo da fase do protocolo
 * @param period fase do protocolo de ligação de dados
 * @param status interviniente que espera a frame
 * @return FALSE se não é o campo de cotnrolo esperado e TRUE se é o esperado
*/
int expectedControl(Period period, Status status, unsigned char ch);


/**
 *  Envio de trama de supervisão/não numerada(SET, UA, RR, REJ, DISC)
 * @param fd descritor da porta de série
 * @param control campo de controlo da trama
 * @return 0 em caso de sucesso e -1 em caso de falha
*/
int sendSupervisionFrame(int fd, Control control);


// Tranferencia de Dados - leitura da porta de serie

/**
 * Leitura da informação da porta de série
 * @param fd descritor da porta de série
 * @param buffer para armazenar os dados lidos da porta de série
 * @return comprimento do array(número de carateres lidos) ou -1 em caso de falha
*/
int llread(int fd, unsigned char* buffer);


/**
 * Receção da trama de informação(I)
 * @param fd descritor da porta de série
 * @return número de bytes de dados se não há erros no cabeçalho (BCC1) nem no campo de dados(BCC2) e -1 no caso de erros no campo de dados(BCC2)
*/
int receiveInfoFrame(int fd);


/**
 * Determina resposta a enviar ao emissor da trama, consoante o número de série e a validade do campo de dados
 * @param validDataField indica se o campo de dados recebido tem erros
 * @param expectedSequenceNumber indica se o número de série recebido é o esperado
 * @return 0(TRUE) se o número de série é o esperado e 1(FALSE) caso contrário
*/
Control buildAck(int validDataField, int expectedSequenceNumber);

// Tranferencia de Dados - escrita na porta de serie

/**
 * Escrita de um pacote de dados na porta de série
 * @param fd descritor da porta de série
 * @param buffer onde é recebido o pacote de dados
 * @return número de carateres escritos ou -1 em caso de falha
*/
int llwrite(int fd, unsigned char* buffer, int length);

/**
 * Criação da frame I 
 * @param controlField C_N0 ou C_N1, consoante o número de série 
 * @param infoField campo de dados
 * @param infoFieldLength tamanho do campo de dados
 * @return
*/
int createFrameI(Control controlField, unsigned char* infoField, int infoFieldLength);

/**
 * Escrita da frame I na porta de série
 * @param fd descritor da porta de série
 * @param length tamanho da trama I
 * @return
*/
int sendFrameI(int fd, int length);

// Byte Stuffing e Destuffing

// TODO
int byte_stuffing( int length);


/**
 * Aplica o mecanismo de byte destuffing para recuperar os octetos originais (antes da operação de stuffing)
*/
int byteDestuffing();

// TODO - terminacao com envio e rececao de DISC; para ja so liberta a memoria e restaura configuracao da porta
int llclose(int fd);

#endif
