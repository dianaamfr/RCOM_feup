#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include "datalink.h"
#include "utils.h"
  
extern unsigned int resend;

int main(int argc, char** argv) {

  /*
  if ( (argc < 2) || ((strcmp("/dev/ttyS0", argv[1])!=0) && (strcmp("/dev/ttyS1", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }
  */

  struct termios oldtio;

  int fd = openNonCanonical(argv[1],&oldtio);

  receiveControl(fd, C_SET); /* Espera por trama SET*/

  resend = FALSE;
  sendControl(fd, C_UA); /* Envia resposta UA para a porta de serie */
  
  restoreConfiguration(fd, &oldtio);
  
  return 0;
}
