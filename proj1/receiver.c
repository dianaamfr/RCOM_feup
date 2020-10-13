#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "receiver.h"
#include "datalink.h"

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

  int fd;
  struct termios oldtio, newtio;

  /*
  if ( (argc < 2) || ((strcmp("/dev/ttyS0", argv[1])!=0) && (strcmp("/dev/ttyS1", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }
  */

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
    
  fd = open(argv[1], O_RDWR | O_NOCTTY );
  if (fd < 0) {
    perror(argv[1]);
    exit(-1);
  }

  if (tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  newtio.c_lflag = 0; /* set input mode (non-canonical, no echo,...) */

  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) proximo(s) caracter(es).

    VTIME = 0 & VMIN = 0 read will be satisfied immediately. The number of characters currently
    available, or the number of characters requested will be returned.
  */
  newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 0;  /* sets the minimum number of characters to receive before satisfying the read. */

  tcflush(fd, TCIOFLUSH);

  if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");

  receiveControl(fd, C_SET); /* Espera por trama SET*/

  sendUa(fd); /* Envia resposta UA para a porta de serie */
  
  
  tcsetattr(fd,TCSANOW,&oldtio);
  close(fd);
  return 0;
}
