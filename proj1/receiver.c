#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "datalink.h"
#include "utils.h"
#include "app.h"
  
extern unsigned int resend;

int main(int argc, char** argv) {

  // Pode ser lido apenas o número da porta?
  /*if(validateArgs(argc, argv) == -1) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    return -1;
  }*/

  if ( (argc < 2)/* ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) &&
          (strcmp("/dev/ttyS2", argv[1])!=0) &&
          (strcmp("/dev/ttyS3", argv[1])!=0) &&
          (strcmp("/dev/ttyS4", argv[1])!=0) )*/) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }
  
  // A partir daqui será feito na app provavelmente

  if(receiveFile(argv[1])<0){
      return -1;
  }


  /*if((fd = llopen(argv[1], RECEIVER)) < 0){
    perror("Couldn't connect with Transmitter");
    return -1;
  }

  // Usado para testar a receção da trama de info pelo recetor e envio de RR
  unsigned char buf[20];
  unsigned char buf1[20];
  int nr = llread(fd, buf);

  printf("Bytes read from port: \n");
  for(int i = 0; i < nr; i++){
    printf("%4X",buf[i]);
  }

  printf("\n");

  nr = llread(fd, buf1);

  printf("Bytes read from port: \n");
  for(int i = 0; i < nr; i++){
    printf("%4X",buf1[i]);
  }

  printf("\n");

  llclose(fd, RECEIVER);*/

  return 0;
}
