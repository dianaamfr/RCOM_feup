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
#include <signal.h>


#define BAUDRATE          B38400
#define _POSIX_SOURCE     1  /* POSIX compliant source */

int cnt=1;
int flag=1;

void alarmhandler(){
	if(cnt <= 3){
    printf("#%d: Return message not received, waiting 3 more seconds..\n", cnt);
    flag=1;
    cnt++;
  }
  else{
    printf("[EXITING]\n Remote App couldnt establish communication, aborting\n");
    exit(-1);
  }
  alarm(3);
}

int main(int argc, char** argv) {

  signal(SIGALRM,alarmhandler);

  int fd,c, res;
  struct termios oldtio,newtio;
  int i, sum = 0, speed = 0;

  unsigned char setMsg[5]; /* trama SET */
    
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
  newtio.c_cc[VMIN] = 1;  /* sets the minimum number of characters to receive before satisfying the read. */


  tcflush(fd, TCIOFLUSH);

  if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }
  
  printf("New termios structure set\n");


  /* TODO - Definir e enviar trama SET para a porta de serie : */

  enum address add;
  enum control ctrl;

  char SET[5] = {FLAG,FLAG};


  res = write(fd, SET, sizeof(SET));
  printf("%d bytes written\n", res);
 
  int count = 0; 
  alarm(3);          







  if (tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
  return 0;
}
