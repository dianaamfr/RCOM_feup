#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../datalink.h"
#include "../utils.h"
#include "../app.h"
  
extern unsigned int resend;

int main(int argc, char** argv) {

  if(validateArgs(argc, argv) == -1) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    return -1;
  }

  if(receiveFile(argv[1])<0){
      return -1;
  }

  return 0;
}
