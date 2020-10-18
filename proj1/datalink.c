#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include "datalink.h"
#include <signal.h>
#include <string.h>
#include "alarm.h"
#include "utils.h"
#include <stdbool.h>


int llopen(int port, Status status){

  int fd;
  struct termios oldtio;

  initDataLink(port);

  fd = openNonCanonical(dataLink->port,&oldtio);
  if(fd == -1){
    restoreConfiguration(fd, &oldtio);
    return -1;
  }

  tries = 0;     
  resend = FALSE; 

  switch (status)
  {
    case RECEIVER:
      if(openReceiver(fd) == -1){
        restoreConfiguration(fd, &oldtio);
        return -1;
      }
      break;
    
    case TRANSMITTER:
      if(openTransmitter(fd) == -1){
        restoreConfiguration(fd, &oldtio);
        return -1;
      }
      break;
      
    default:
      perror("Invalid Status: nor RECEIVER nor TRANSMITTER");
      restoreConfiguration(fd, &oldtio);
      return -1;
  }

  return fd;
}


int initDataLink(int port) {

	dataLink = (linkLayer*) malloc(sizeof(linkLayer));
  if(dataLink == NULL){
    perror("Malloc Link Layer struct");
    return -1;
  }

	if( sprintf(dataLink->port, "/dev/ttyS%d", port) < 0 ){
    perror("Sprintf: copying port name error");
    return -1;
  }

	dataLink->baudRate = BAUDRATE;
	dataLink->sequenceNumber = 0; // Começar com número de sequência 0
  dataLink->timeout = TIMEOUT;
  dataLink->numTransmissions = MAX_RETR;

  return 0;
}


int openReceiver(int fd) {

  if(receiveSupervisionFrame(fd, C_SET) == -1) { /* Espera por trama SET*/
    perror("Error receiving SET frame");
    return -1;
  }

  resend = FALSE;
  if(sendSupervisionFrame(fd, C_UA) == -1) { /* Envia UA para a porta de serie */
    perror("Error sending UA frame");
    return -1;
  }

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

    if (receiveSupervisionFrame(fd, C_UA) == 0){
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
        return 0;
    }
  }
  
  return -1;

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


int llread(int fd, unsigned char* buffer){

  int dataFieldSize = receiveInfoFrame(fd); // Recebe trama de informação
  int validDataField = TRUE;
  
  if(dataFieldSize == -1) 
    validDataField = FALSE;

 /* int expectedSequenceNumber = isExpectedSequenceNumber();

  
  Control ack = buildAck(validDataField, expectedSequenceNumber);

  if(sendSupervisionFrame(fd, ack) == -1 ){
    fprintf(stderr,"Error sending %s\n", getControlName(ack));
    return -1;
  }

  if(validDataField == TRUE)
    memcpy(buffer,&dataLink->frame[HEADER_SIZE], dataFieldSize);*/

  return dataFieldSize;

}


int receiveInfoFrame(int fd) {
 
  State iState = START;
  unsigned char ch, bcc1 = 0;
  int nr, i = 0; 
  int end = FALSE; 

  printf("Bytes read: \n");

  while(!end) {

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
          memset(dataLink->frame, 0, sizeof(dataLink->frame)); 

          iState = FLAG_RCV;
          dataLink->frame[i] = ch;
          i++;
        }
        break;

      case FLAG_RCV:
        if (ch == A){
          bcc1 ^= ch;

          iState = A_RCV;
          dataLink->frame[i] = ch;
          i++;
        }
        else if (ch != FLAG)
          iState = START;
        break;

      case A_RCV:
        if (isInfoSequenceNumber(ch) == TRUE){
          bcc1 ^= ch;

          iState = C_RCV;
          dataLink->frame[i] = ch;
          i++;
        }
        else if(ch != FLAG){
          iState = START;
        }  
        else{
          iState = FLAG_RCV;
          i = 1;
        }
        break;

      case C_RCV:
        if (ch == FLAG){
          iState = FLAG_RCV;
          i = 1;
        }
        else if (ch == bcc1){
          iState = BCC_OK;
          dataLink->frame[i] = ch;
          i++;
        }
        else
          iState = START;  
        break;

      case BCC_OK:
        dataLink->frame[i] = ch;
        i++;
        if(ch == FLAG){
          iState = STOP;
        }
        break;

      case STOP:
        printf("\nReceived Information Frame with success\n");
        end = TRUE;
    
    }
  }
  
  int dataSize = i - DELIMIT_INFO_SIZE;
  if(validBcc2(&dataLink->frame[HEADER_SIZE], dataSize + 1) != -1)
    return dataSize;

  return -1; // Erro no BCC2

}


int validBcc2(unsigned char * dataField, int length) {
  unsigned char bcc_received = dataField[length - 1];
  unsigned char bcc_calculated = 0;

  for(int i = 0; i < length - 1; i++){ // XOR dos bytes de dados
    bcc_calculated ^= dataField[i];
  }

  if(bcc_calculated == bcc_received)
    return 0;
  
  return -1;
}


int isInfoSequenceNumber(unsigned char byte){
  if(byte == I_0 || byte == I_1)
    return TRUE;
  return FALSE; 
}


int isExpectedSequenceNumber(){
  int receivedSequenceNumber = (dataLink->frame[CONTROL_BYTE] >> 7) & 0x01;

  if(receivedSequenceNumber == dataLink->sequenceNumber)
    return TRUE;
  return FALSE;
}


Control buildAck(int validDataField, int expectedSequenceNumber){

  if(validDataField == TRUE){ // Tramas sem erros nos dados

    if(expectedSequenceNumber == TRUE) // Nova Trama
      dataLink->sequenceNumber = !dataLink->sequenceNumber; // Passa a pedir o outro Numero de Sequencia
      
    if(dataLink->sequenceNumber == 0)
      return C_RR_0;
    return C_RR_1;
  }

  // Tramas com erros nos dados
  if(expectedSequenceNumber == TRUE){ // Nova Trama - pedido de retransmissao
    if(dataLink->sequenceNumber == 0)
      return C_REJ_0;
    return C_REJ_1;
  }

  return C_RR_0;  // Trama repetida com erros nos dados

}




unsigned char createBCC(unsigned char a, unsigned char c) {
    return a ^ c;
}

unsigned char createBCC_2(unsigned char* frame, int length) {

  unsigned char bcc2 = frame[0];

  for(int i = 1; i < length; i++){
    bcc2 = bcc2 ^ frame[i];
  }

  return bcc2;
}

int createFrameI(unsigned char* frame, unsigned char controlField, unsigned char* infoField, int infoFieldLength) {

  frame[0] = FLAG;

  frame[1] = 0x03; 

  frame[2] = controlField;

  frame[3] = createBCC(frame[1], frame[2]);

  for(int i = 0; i < infoFieldLength; i++) {
    frame[i + 4] = infoField[i];
  }

  unsigned bcc2 = createBCC_2(infoField, infoFieldLength);

  frame[infoFieldLength + 4] = bcc2;

  frame[infoFieldLength + 5] = FLAG;

  return 0;
}

int sendFrameI(unsigned char* frame, int fd, int length) {

    int n;
    if( (n = write(fd, frame, length)) <= 0){
        return -1;
    }
    return n;
}





int llwrite(int fd, char* buffer, int length) {

  unsigned char controlByte;
  unsigned char answer_buffer[MAX_DATA_FIELD];

  if(&dataLink->sequenceNumber == 0)
    controlByte = C_N0;
  else controlByte = C_N1;


  if (createFrameI(dataLink->frame, controlByte,(unsigned char*) buffer, length) != 0) {
    free(&dataLink->frame);
    //close
    return -1;
  }

  printf("AAAAAAAAAAA");


/*

  int numWritten;
  bool dataSent = false;
  
  while(!dataSent) {

    if((numWritten = sendFrameI(dataLink->frame, fd, length)) == -1) {
      free(&dataLink->frame);
      //closeNonCanonical(fd, &oldtio);
      return -1;
    }

    printf("Sent I frame\n");

    int read_value = -1;
    int finish = 0;
    int num_retr = 0;
  
    alarm(dataLink->timeout);
  
    unsigned char wantedBytes[2];
  
    if (controlByte == C_N0) {
      wantedBytes[0] = C_RR_1;
      wantedBytes[1] = C_REJ_0;
    }
    else if (controlByte == C_N0) {
      wantedBytes[0] = C_RR_0;
      wantedBytes[1] = C_REJ_1;
    }
  
    while (finish != 1) {
      read_value = 0;
      if(read_value >= 0) { // read_value é o índice do wantedByte que foi encontrado
        // Cancels alarm
        alarm(0);
        finish = 1;
      }
    }

    if(read_value == -1){
      printf("Closing file descriptor\n");
      free(&dataLink->frame);
      //closeNonCanonical(fd, &oldtio);
      return -1;
    }

    if(read_value == 0) // read a RR
      dataSent = true;
    else // read a REJ
      dataSent = false;
  

    printf("Received response frame ");
  }


  if (dataLink->sequenceNumber == 0)
    dataLink->sequenceNumber = 1;
  else if (dataLink->sequenceNumber == 1)
    dataLink->sequenceNumber = 0;
  else return -1;
  
  */

  int numWritten;
  if((numWritten = sendFrameI(dataLink->frame, fd, length)) == -1) {
      free(&dataLink->frame);
      //closeNonCanonical(fd, &oldtio);
      return -1;
  }

  printf("OKKKKKKKKKKKKKKKKKKK");


  return (numWritten - 6); // length of the data packet length sent to the receiver
}
