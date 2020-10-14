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

  char * port = (char*)malloc(3*sizeof(char));

  if(validateArgs(argc, argv) == -1) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    return -1;
  }

  strcpy(port,&argv[1][9]);

  if(llopen(atoi(port),TRANSMITTER)){
    perror("llopen");
    return -1;
  }
  
  free(port); 
  return 0;
}
