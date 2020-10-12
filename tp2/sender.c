#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"
#include <signal.h>


#define BAUDRATE          B38400
#define _POSIX_SOURCE     1  /* POSIX compliant source */
#define TIMEOUT           3
#define MAX_RETR          3

#define A       0x03         /* Campo de Endereço em Respostas enviadas pelo Receptor */
#define C_SET   0x03         /* Set up control */
#define FLAG    0x7E         /* Flag que delimita as tramas */

int tentativas=0, fd;


/* Envio da trama SET */
void sendSet(int fd) {
  unsigned char SET[5]; /* trama SET */
  int n;
  
  SET[0] = FLAG;
  SET[1] = A;
  SET[2] = C_SET;
  SET[3] = SET[1] ^ SET[2];
  SET[4] = FLAG;

  for(int i = 0; i < sizeof(SET);) {
    n = write(fd, SET, sizeof(SET));
    i += n;
    printf("Bytes sent: %d/%zu.\n", i, sizeof(SET));
  }
  for (int i = 0; i < 5; i++){  
      printf("%4X ", SET[i]);
  }
  printf("\n");

}

void alarmhandler(){
  if(tentativas < MAX_RETR){
    sendSet(fd);
    printf("Didnt receive, waiting 3 seconds..\n");
    alarm(3);
  }
  else{
    printf("Aborting\n");
    exit(-1);
  }
  tentativas++;
}


int check_protection(char trama[]){
  char bcc = trama[1] ^ trama[2];
  if(bcc == trama[3]) return TRUE;
  else return FALSE;
}


int main(int argc, char** argv) {

  signal(SIGALRM,alarmhandler);

  int  c, res;
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
  newtio.c_cc[VMIN] = 1;  /* sets the minimum number of characters to receive before satisfying the read. */


  tcflush(fd, TCIOFLUSH);

  if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }
  
  printf("New termios structure set\n");
  
  /* TODO Implementar o mecanismo de retransmissão, do lado do Emissor, com time-out. */
  
  /* Enviar trama SET para a porta de serie */
 // sendSet(fd);

  /* TODO - receiveUa(fd);*/

  alarm(TIMEOUT);

  int n;
  int count=0;
  unsigned char buf[255];

  char trama[255];
  bzero(trama, sizeof(trama));
  int pos=0;

  while(count<5){
    n=read(fd,buf,1);
    buf[n]=0;
    trama[pos]=buf[0];
    count+=1;
  }

  printf("UA:");
  for (int i = 0; i < 5; i++){
      printf("%4X ", trama[i]);
  }
  printf("received\n");

  if(check_protection(trama)==1){
    printf("BBC check error\n");
    exit(-1);
  }
  else{
    printf("BBC CORRECT\n");
  }

  sleep(2);

  if (tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
  return 0;
}
