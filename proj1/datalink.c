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

  receiveSupervisionFrame(fd, C_SET); /* Espera por trama SET*/

  resend = FALSE;
  sendSupervisionFrame(fd, C_UA); /* Envia UA para a porta de serie */

  return 0;
};


int openTransmitter(int fd) {

  signal(SIGALRM,alarmHandler); // Instala rotina que atende interrupcao do alarme

  tries = 0;     
  resend = FALSE; 

  /* (Re)transmissao da trama SET */
  while(tries < MAX_RETR){
    
    sendSupervisionFrame(fd, C_SET);

    resend = FALSE; 
    alarm(TIMEOUT); /* Inicia espera por UA */

    if (receiveSupervisionFrame(fd, C_UA) == TRUE){
      resend = FALSE; 
      alarm(0);
      return 0;
    } 

  }

  return -1;
};


int receiveSupervisionFrame(int fd, Control control) {

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


int sendSupervisionFrame(int fd, Control control) {
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


int receiveInfoFrame(int fd, Control control) {
  // TODO - em desenvolvimento
  State iState = START;
  unsigned char ch, bcc1 = 0;
  int nr, i = 0;  

  printf("Bytes read: \n");

  while(TRUE) {

    if(iState != STOP){
      nr = read(fd, &ch, 1);
      if(nr > 0)
        printf("%4X",ch);
    }                  

    switch(iState) {
      case START:
        if (ch == FLAG){
          bcc1 = 0;
          i = 0;
          iState = FLAG_RCV;
        }
        else
          iState = START;
        break;

      case FLAG_RCV:
        if (ch == A){
          iState = A_RCV;
          bcc1 ^= ch;
        }
        else if (ch != FLAG)
          iState = START;
        break;

      case A_RCV:
        if (ch == control){
          iState = C_RCV;
          bcc1 ^= ch;
        }
        else if (ch != FLAG)
          iState = START;
        else
          iState = FLAG_RCV;
        break;

      case C_RCV:
        if (ch == FLAG)
          iState = FLAG_RCV;
        else if (ch == bcc1)
          iState = BCC_OK;
        else
          iState = START;  
        break;

      case BCC_OK:
        if(ch == FLAG && validBcc2(dataLink.frame,i+1)) {
          iState = STOP;
        }
        else{
          dataLink.frame[i] = ch;
          i++;
        }
        break;

      case STOP:
        printf("\nReceived %s message with success\n", getControlName(control));
        return TRUE;
    }
  }
  
  return FALSE;

}

int validBcc2(unsigned char * dataField, int length){
  unsigned char bcc_received = dataField[length-1];
  unsigned char bcc_calculated = 0;

  for(int i = 0; i < length - 1; i++){
    bcc_calculated ^= dataField[i];
  }

  return bcc_calculated == bcc_received;
}
