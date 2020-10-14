#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "datalink.h"
#include "utils.h"
  
extern unsigned int resend;

int main(int argc, char** argv) {

  char * port = (char*)malloc(3*sizeof(char));

  if(validateArgs(argc, argv) == -1) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    return -1;
  }

  strcpy(port,&argv[1][9]);

  if(llopen(atoi(port),RECEIVER)){
    perror("llopen");
    return -1;
  }
  
  free(port);
  return 0;
}
