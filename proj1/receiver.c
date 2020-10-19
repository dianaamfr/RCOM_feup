#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "datalink.h"
#include "utils.h"
  
extern unsigned int resend;

int main(int argc, char** argv) {

  int port, fd;

  // Pode ser lido apenas o número da porta?
  if(validateArgs(argc, argv) == -1) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    return -1;
  }
  
  port = atoi(&argv[1][9]);

  // A partir daqui será feito na app provavelmente
  if((fd = llopen(port, RECEIVER)) < 0){
    perror("llopen Receiver");
    return -1;
  }

  /* Usado para testar a receção da trama de info pelo recetor e envio de RR*/
  unsigned char buf[20];
  unsigned char buf1[20];
  int nr = llread(fd, buf);

  printf("Bytes read from port: \n");
  for(int i = 0; i < nr; i++){
    printf("%4X",buf[i]);
  }

  nr = llread(fd, buf1);

  printf("Bytes read from port: \n");
  for(int i = 0; i < nr; i++){
    printf("%4X",buf1[i]);
  }

  printf("\n");

  llclose(fd);

  return 0;
}
