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

#define A  0x03           /* Campo de Endereço em Comandos enviados pelo Emissor */
#define C_UA  0x07        /* Unnumbered Acknowledgment control */
#define C_SET   0x03      /* Set up control */
#define FLAG  0x7E        /* Flag que delimita as tramas */


/**
 *  Rececao da trama SET
 * 
 * @param fd descritor da porta de serie
*/
void receiveSet(int fd) {

  state setState = START;
  unsigned char ch, bcc = 0;
  int nr;  

  while(TRUE) {

    if(setState != OTHER_RCV && setState != STOP){
      nr = read(fd, &ch, 1);
      if(nr > 0)
        printf("Byte read: %x\n",ch);
    }                  

    switch(setState) {
      case START:
        if (ch == FLAG){
          bcc = 0;
          setState = FLAG_RCV;
        }
        else
          setState = OTHER_RCV;
        break;

      case FLAG_RCV:
        if (ch == A){
          setState = A_RCV;
          bcc ^= ch;
        }
        else if (ch != FLAG)
          setState = OTHER_RCV;
        break;

      case A_RCV:
        if (ch == C_SET){
          setState = C_RCV;
          bcc ^= ch;
        }
        else if (ch != FLAG)
          setState = OTHER_RCV;
        else
          setState = FLAG_RCV;
        break;

      case C_RCV:
        if (ch == FLAG)
          setState = FLAG_RCV;
        else if (ch == bcc)
          setState = BCC_OK;
        else
          setState = OTHER_RCV;  
        break;

      case BCC_OK:
        if(ch == FLAG) 
          setState = STOP;
        break;

      case OTHER_RCV:
        setState = START;
        break;

      case STOP:
        printf("Received SET message with success\n");
        return;
    }
  }
}


/**
 *  Envio da trama UA
 * 
 * @param fd descritor da porta de serie
*/
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

  receiveSet(fd); /* Espera por trama SET*/
  
  sendUa(fd); /* Envia resposta UA para a porta de serie */
  
  
  tcsetattr(fd,TCSANOW,&oldtio);
  close(fd);
  return 0;
}
