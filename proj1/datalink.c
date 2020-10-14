#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include "datalink.h"
#include <signal.h>
#include "alarm.h"
#include "utils.h"


int llopen(int port, Status status){

  int fd;
  char* portName = (char*)malloc(sizeof(char)*sizeof("/dev/ttySx"));
  struct termios oldtio;

  sprintf(portName, "/dev/ttyS%d",port);

  fd = openNonCanonical(portName,&oldtio);
  if(fd == -1) 
    return -1;

  tries = 0;     
  resend = FALSE; 

  switch (status)
  {
  case RECEIVER:
    openReceiver(fd);
    break;
  
  case TRANSMITTER:
    openTransmitter(fd);
    break;
    
  default:
    perror("Status");
    restoreConfiguration(fd, &oldtio);
    return -1;
  }

  restoreConfiguration(fd, &oldtio);
  return 0;
}


int openReceiver(int fd) {

  receiveControl(fd, C_SET); /* Espera por trama SET*/

  resend = FALSE;
  sendControl(fd, C_UA); /* Envia UA para a porta de serie */

  return 0;
};


int openTransmitter(int fd) {

  signal(SIGALRM,alarmHandler); // Instala rotina que atende interrupcao do alarme

  tries = 0;     
  resend = FALSE; 

  /* (Re)transmissao da trama SET */
  while(tries < MAX_RETR){
    
    sendControl(fd, C_SET);

    resend = FALSE; 
    alarm(TIMEOUT); /* Inicia espera por UA */

    if (receiveControl(fd, C_UA) == TRUE){
      resend = FALSE; 
      alarm(0);
      return 0;
    } 

  }

  return -1;
};


int receiveControl(int fd, Control control) {

  State uaState = START;
  unsigned char ch, bcc = 0;
  int nr;  

  printf("Bytes read: \n");

  while(resend == FALSE) {

    if(uaState != STOP){
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
          uaState = START;
        break;

      case FLAG_RCV:
        if (ch == A){
          uaState = A_RCV;
          bcc ^= ch;
        }
        else if (ch != FLAG)
          uaState = START;
        break;

      case A_RCV:
        if (ch == control){
          uaState = C_RCV;
          bcc ^= ch;
        }
        else if (ch != FLAG)
          uaState = START;
        else
          uaState = FLAG_RCV;
        break;

      case C_RCV:
        if (ch == FLAG)
          uaState = FLAG_RCV;
        else if (ch == bcc)
          uaState = BCC_OK;
        else
          uaState = START;  
        break;

      case BCC_OK:
        if(ch == FLAG) 
          uaState = STOP;
        break;

      case STOP:
        printf("\nReceived %s message with success\n", getControlName(control));
        return TRUE;
    }
  }
  
  return FALSE;

}


int sendControl(int fd, Control control) {
  unsigned char buf[5];
  int nw;
  
  buf[0] = FLAG;
  buf[1] = A;
  buf[2] = control;
  buf[3] = buf[1] ^ buf[2];
  buf[4] = FLAG;

  tcflush(fd, TCIOFLUSH);

  nw = write(fd, buf, sizeof(buf));
  if (nw != sizeof(buf)){
    fprintf(stderr,"Error writing %s\n", getControlName(control));
    return -1;
  }

  printf("Sent %s message with success\n", getControlName(control));

  return 0;

}
