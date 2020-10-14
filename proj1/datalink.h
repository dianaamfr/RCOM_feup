#ifndef DATALINK_HEADER
#define DATALINK_HEADER

#include "macros.h"

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
int receiveControl(int fd, Control C);

int sendControl(int fd, Control C);

#endif
