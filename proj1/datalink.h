#ifndef DATALINK
#define DATALINK

#include "macros.h"

/**
 *  Maquina de estados para receber uma trama de controlo 
 * 
 * @param fd descritor da porta de serie
*/
int receiveControl(int fd, control C);

#endif
