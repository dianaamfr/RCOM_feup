#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"


#define BAUDRATE          B38400
#define _POSIX_SOURCE     1  /* POSIX compliant source */

#define A       0x03         /* Campo de Endereço em Respostas enviadas pelo Receptor */
#define C_SET   0x03         /* Set up control */
#define FLAG    0x7E         /* Flag que delimita as tramas */

/* Envio da trama SET */
void sendSet(int fd) {
  unsigned char buf[5]; /* trama SET */
  int n;
  
  buf[0] = FLAG;
  buf[1] = A;
  buf[2] = C_SET;
  buf[3] = buf[1] ^ buf[2];
  buf[4] = FLAG;

  for(int i = 0; i < sizeof(buf);) {
    n = write(fd, buf, sizeof(buf));
    i += n;
    printf("Bytes sent: %d/%zu.\n", i, sizeof(buf));
  }

}

int main(int argc, char** argv) {

  int fd,c, res;
  struct termios oldtio,newtio;
  int i, sum = 0, speed = 0;
    
  /*if ( (argc < 2) || 
        ((strcmp("/dev/ttyS0", argv[1])!=0) && 
        (strcmp("/dev/ttyS1", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }*/

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

  fd = open(argv[1], O_RDWR | O_NOCTTY );
  if (fd <0) {perror(argv[1]); exit(-1); }

  if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) proximo(s) caracter(es).

    VTIME = 0 & VMIN = 0 read will be satisfied immediately. The number of characters currently
    available, or the number of characters requested will be returned.
  */

  newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 0;  /* sets the minimum number of characters to receive before satisfying the read. */


  tcflush(fd, TCIOFLUSH);

  if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }
  

  printf("New termios structure set\n");
  
  /* TODO Implementar o mecanismo de retransmissão, do lado do Emissor, com time-out. */
  
  sendSet(fd);/* Enviar trama SET para a porta de serie */

  /* TODO - receiveUa(fd);*/

  if (tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
  return 0;
}
