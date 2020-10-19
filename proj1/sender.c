#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "datalink.h"
#include "utils.h"


int main(int argc, char** argv) {

  int port, fd;

  // Pode ser lido apenas o número da porta?
  if(validateArgs(argc, argv) == -1) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    return -1;
  }

  port = atoi(&argv[1][9]);
  
  // A partir daqui será feito na app provavelmente
  if((fd = llopen(port, TRANSMITTER)) < 0){
    perror("llopen Transmitter");
    return -1;
  }

  /* Usado para testar a receção da trama de info pelo recetor e receção de RR pelo emissor*/
  printf("Send Info Frame\n");

  unsigned char buffer[20];
  buffer[0] = 0x17;
  buffer[1] = 0x30;
  buffer[2] = 0x16;
  buffer[3] = 0x93;
  buffer[4] = 0x50;
  buffer[5] = 0x77;


  if(llwrite(fd, buffer, 6) < 0) {
    printf("deu erro");
    return -1;
  }

  if(llwrite(fd, buffer, 6) < 0) {
    printf("deu erro");
    return -1;
  }

  llclose(fd);
  
  return 0;
}