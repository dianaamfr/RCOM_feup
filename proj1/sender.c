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

 extern unsigned int tries;     
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
  if((fd = llopen(port, TRANSMITTER)) < 0){
    perror("llopen Transmitter");
    return -1;
  }
  
  return 0;
}
