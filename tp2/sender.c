#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "types.h"
#include "utils.h"


#define BAUDRATE          B38400
#define _POSIX_SOURCE     1  /* POSIX compliant source */

#define TIMEOUT           3   
#define MAX_RETR          3   

#define A       0x03         /* Campo de Endereço em Respostas enviadas pelo Receptor */
#define C_UA    0x07        /* Unnumbered Acknowledgment control */
#define C_SET   0x03         /* Set up control */
#define FLAG    0x7E         /* Flag que delimita as tramas */

unsigned int tries = 0;     /* Numero de tentativas usadas para retransmissao do comando SET e espera do UA */
unsigned int alarmWentOff = FALSE; /* Flag para assinalar se o alarme foi disparado porque passou o tempo TIMEOUT */

/**
 * Handler para o sinal SIGALARM
 * 
 * Assinala que o alarme foi acionado através da flag alarmWentOff e incrementa o numero de tentativas
 * 
 * @param sig sinal recebido
*/
void alarmhandler(int sig){
  tries++;
  alarmWentOff = TRUE; /* Assinala que o alarme foi accionado */

  printf("\nTimeout! Tries used: %d \n", tries);
  return;
}


/**
 *  Envio da trama SET
 * 
 * @param fd descritor da porta de serie
*/
void sendSet(int fd) {
  unsigned char SET[5]; /* trama SET */
  int n;
  
  SET[0] = FLAG;
  SET[1] = A;
  SET[2] = C_SET;
  SET[3] = SET[1] ^ SET[2];
  SET[4] = FLAG;

  tcflush(fd, TCIOFLUSH);

  for(int i = 0; i < sizeof(SET);) {
    n = write(fd, SET, sizeof(SET));
    i += n;
    printf("Bytes sent: %d/%zu.\n", i, sizeof(SET));
  }

  for (int i = 0; i < sizeof(SET); i++){  
      printf("%4X ", SET[i]);
  }
  printf("\n");
  
}


/**
 *  Maquina de estados para receber a trama UA lida da porta de serie 
 * 
 * @param fd descritor da porta de serie
*/
int receiveUa(int fd) {

  state uaState = START;
  unsigned char ch, bcc = 0;
  int nr;  

  printf("Bytes read: \n");

  while(alarmWentOff == FALSE) {

    if(uaState != OTHER_RCV && uaState != STOP){
      nr = read(fd, &ch, 1);
      if(nr > 0)
        printf("%4X",ch);
    }                  

    switch(uaState) {
      case START:
        if (ch == FLAG){
          bcc = 0;
          uaState = FLAG_RCV;
        }
        else
          uaState = OTHER_RCV;
        break;

      case FLAG_RCV:
        if (ch == A){
          uaState = A_RCV;
          bcc ^= ch;
        }
        else if (ch != FLAG)
          uaState = OTHER_RCV;
        break;

      case A_RCV:
        if (ch == C_UA){
          uaState = C_RCV;
          bcc ^= ch;
        }
        else if (ch != FLAG)
          uaState = OTHER_RCV;
        else
          uaState = FLAG_RCV;
        break;

      case C_RCV:
        if (ch == FLAG)
          uaState = FLAG_RCV;
        else if (ch == bcc)
          uaState = BCC_OK;
        else
          uaState = OTHER_RCV;  
        break;

      case BCC_OK:
        if(ch == FLAG) 
          uaState = STOP;
        break;

      case OTHER_RCV:
        uaState = START;
        break;

      case STOP:
        printf("\nReceived UA message with success\n");
        return TRUE;
    }
  }
  
  return FALSE;
}


int main(int argc, char** argv) {

  signal(SIGALRM,alarmhandler); // Instala rotina que atende interrupcao do alarme

  int fd, c, res;
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

  if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }


  printf("New termios structure set\n");

  while(tries < MAX_RETR){ /* Enquanto nao se tiverem esgotado as tentativas */
    
    sendSet(fd); /* Transmite/Retransmite trama SET */

    alarmWentOff = FALSE; /* A Flag passa a indicar que o tempo ainda nao se esgotou */
    alarm(TIMEOUT); /* Ativacao do alarme para esperar por UA válida */

    if (receiveUa(fd) == TRUE){ /* Ao receber a trama UA desativa o alarme e termina */
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
