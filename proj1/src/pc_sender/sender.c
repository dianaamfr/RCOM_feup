#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../datalink.h"
#include "../utils.h"
#include "../app.h"


int main(int argc, char** argv) {

  if(validateArgs(argc, argv) == -1) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    return -1;  
  }

  if(sendFile(argv[1]) < 0){
      return -1;
  }

  return 0;
}