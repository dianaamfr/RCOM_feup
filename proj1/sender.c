#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "sender.h"
#include "datalink.h"
#include "alarm.h"

 extern unsigned int tries;     
 extern unsigned int resend; 

void sendSet(int fd) {
  unsigned char SET[5]; /* trama SET */
  int nw;
  
  SET[0] = FLAG;
  SET[1] = A;
  SET[2] = C_SET;
  SET[3] = SET[1] ^ SET[2];
  SET[4] = FLAG;

  tcflush(fd, TCIOFLUSH);

  nw = write(fd, SET, sizeof(SET));
  if (nw != sizeof(SET))
		perror("Error writing SET\n");
 
  for (int i = 0; i < sizeof(SET); i++){  
      printf("%4X ", SET[i]);
  }
  printf("\n");
  
}

int main(int argc, char** argv) {

  signal(SIGALRM,alarmHandler); // Instala rotina que atende interrupcao do alarme

  int fd;
  struct termios oldtio,newtio;
    
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

  if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }


  printf("New termios structure set\n");

  tries = 0;     
  resend = FALSE; 

  while(tries < MAX_RETR){ /* Enquanto nao se tiverem esgotado as tentativas */
    
    sendSet(fd); /* Transmite/Retransmite trama SET */

    resend = FALSE; /* A Flag passa a indicar que o tempo ainda nao se esgotou */
    alarm(TIMEOUT); /* Ativacao do alarme para esperar por UA válida */

    if (receiveControl(fd, C_UA) == TRUE){ /* Ao receber a trama UA desativa o alarme e termina */
      alarm(0);
      break;
    } 

  }
  

  if (tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
  return 0;
}
