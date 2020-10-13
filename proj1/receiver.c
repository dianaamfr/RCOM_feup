#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include "receiver.h"
#include "datalink.h"
#include "utils.h"

void sendUa(int fd) {
  unsigned char buf[5];
  int nw;
  
  buf[0] = FLAG;
  buf[1] = A;
  buf[2] = C_UA;
  buf[3] = buf[1] ^ buf[2];
  buf[4] = FLAG;

  tcflush(fd, TCIOFLUSH);

  nw = write(fd, buf, sizeof(buf));
  if (nw != sizeof(buf))
		perror("Error writing UA\n");

  printf("Sent UA message with success\n");

}


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

  sendUa(fd); /* Envia resposta UA para a porta de serie */
  
  restoreConfiguration(fd, &oldtio);
  
  return 0;
}
