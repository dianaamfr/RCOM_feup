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
#include "app.h"


int main(int argc, char** argv) {


  // Pode ser lido apenas o número da porta?
  /*if(validateArgs(argc, argv) == -1) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    return -1;  
  }*/

  if ( (argc < 3)/* ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) &&
          (strcmp("/dev/ttyS2", argv[1])!=0) &&
          (strcmp("/dev/ttyS3", argv[1])!=0) &&
          (strcmp("/dev/ttyS4", argv[1])!=0 ))*/) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

  // A partir daqui será feito na app provavelmente

  if(sendFile(argv[1], argv[2])<0){
      return -1;
  }

  /*if((fd = llopen(argv[1], TRANSMITTER)) < 0){
    perror("Couldn't connect with Receiver");
    return -1;
  }

  // Usado para testar a receção da trama de info pelo recetor e receção de RR pelo emissor
  printf("Send Info Frame\n");

  unsigned char buffer[20];
  buffer[0] = ESC;
  buffer[1] = FLAG;
  buffer[2] = 0x65;
  buffer[3] = 0x54;
  buffer[4] = 0x00;

  unsigned char buffer1[20];
  buffer1[0] = 0x98;
  buffer1[1] = 0x11;
  buffer1[2] = 0x05;
  buffer1[3] = 0x5E;


  if(llwrite(fd, buffer, 5) < 0) {
    printf("deu erro");
    return -1;
  }

  if(llwrite(fd, buffer1, 4) < 0) {
    printf("deu erro");
    return -1;
  }

  llclose(fd, TRANSMITTER);*/
  
  return 0;
}