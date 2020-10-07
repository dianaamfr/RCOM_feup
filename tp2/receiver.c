#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "types.h"
#include "utils.h"

#define BAUDRATE          B38400
#define _POSIX_SOURCE     1    /* POSIX compliant source */

/* Maquina de estados para receber a trama SET, lida da porta de serie */
void setReceive(int fd) {

  setRcvState setState = START;
  unsigned char c, bcc;
  int nr;

  while(TRUE) {

    tcflush(fd, TCIOFLUSH);

    nr = read(fd, &c, 1);                  
    printf("c = %c\n", c); /* Mostrar uchar lida*/

    switch(setState) {
      case START:
        if (c == FLAG)
          setState = FLAG_RCV;
        else
          setState = OTHER_RCV;
        break;

      case FLAG_RCV:
        if (c == A_CMD_E){
          setState = A_RCV;
          bcc ^= c;
        }
        else if (c != FLAG)
          setState = OTHER_RCV;
        break;

      case A_RCV:
        if (c == C_SET){
          setState = C_RCV;
          bcc ^= c;
        }
        else if (c != FLAG)
          setState = OTHER_RCV;
        else
          setState = FLAG_RCV;
        break;

      case C_RCV:
        if (c == FLAG)
          setState = FLAG_RCV;
        else if (c == bcc)
          setState = BCC_OK;
        else
          setState = OTHER_RCV;  
        break;

      case BCC_OK:
        if(c == FLAG) 
          setState = STOP;
        break;

      case OTHER_RCV:
        setState = START;
        break;

      case STOP:
        return;
    }
  }

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

  if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");

  setReceive(fd); /* Espera por trama SET*/

  /* TODO - Envia resposta UA para a porta de serie */

  tcsetattr(fd,TCSANOW,&oldtio);
  close(fd);
  return 0;
}
