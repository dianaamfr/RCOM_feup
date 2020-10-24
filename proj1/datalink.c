#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include "datalink.h"
#include <signal.h>
#include <string.h>
#include "alarm.h"
#include "utils.h"

// Estabelecimento da ligação de dados

/*void intHandler(int dummy) {
  alarm(0);
  sleep(4);
  return;
}*/


int llopen(char * port, Status status){
  int fd;

  if(initDataLink(port) == -1){
    return -1;
  }

  fd = openNonCanonical(dataLink->port,&oldtio);
  if(fd == -1){
    restoreConfiguration(fd, &oldtio);
    free(dataLink);
    return -1;
  } 

  signal(SIGALRM,alarmHandler); // Instala rotina que atende interrupcao do alarme
  // signal(SIGINT, intHandler); // Simular Ruído
  
  tries = 0;     
  resend = FALSE; 

  switch (status)
  {
    case RECEIVER:
      if(openReceiver(fd) == -1){
        restoreConfiguration(fd, &oldtio);
        free(dataLink);
        return -1;
      }
      break;
    
    case TRANSMITTER:
      if(openTransmitter(fd) == -1){
        restoreConfiguration(fd, &oldtio);
        free(dataLink);
        return -1;
      }
      break;
      
    default:
      perror("Invalid Status: nor RECEIVER nor TRANSMITTER");
      restoreConfiguration(fd, &oldtio);
      free(dataLink);
      return -1;
  }

  return fd;
}


int initDataLink(char * port) {

	dataLink = (linkLayer*) malloc(sizeof(linkLayer));
  if(dataLink == NULL){
    perror("Malloc Link Layer struct");
    return -1;
  }

  strcpy(dataLink->port, port);

	dataLink->baudRate = BAUDRATE;
	dataLink->sequenceNumber = 0; // Nr sequencia esperado pelo recetor e a enviar pelo emissor

  dataLink->timeout = TIMEOUT;
  dataLink->numTransmissions = MAX_RETR;
  setMaxTries(dataLink->numTransmissions);

  return 0;
}


int openReceiver(int fd) {

  if(receiveSupervisionFrame(fd, SETUP, RECEIVER) == -1) { // Espera por trama SET 
    perror("Error receiving SET frame");
    return -1;
  }

  resend = FALSE;
  if(sendSupervisionFrame(fd, C_UA, RECEIVER) == -1) { // Envia UA para a porta de serie
    perror("Error sending UA frame");
    return -1;
  }

  return 0;
};


int openTransmitter(int fd) {
  
  // (Re)transmissao da trama SET
  while(tries < dataLink->numTransmissions){
    
     if(sendSupervisionFrame(fd, C_SET, TRANSMITTER)== -1){ // Envia SET para a porta de serie
        perror("Error sending SET frame");
        return -1;
     }

    resend = FALSE; 
    alarm(dataLink->timeout); // Inicia espera por UA 

    if (receiveSupervisionFrame(fd, SETUP, TRANSMITTER) != -1){
      resend = FALSE; 
      alarm(0);
      tries = 0;
      return 0;
    }
    // Tenta novamente no caso de falhar a receção de UA
    printf("LinkLayer: Retransmit Set\n");
  }

  printf("LinkLayer: Retransmission attempts to send SET and receive UA exceeded\n");
  return -1;
};


// Receção e envio de tramas de supervisão

int receiveSupervisionFrame(int fd, Period period, Status status) {
 
  State state = START;
  unsigned char ch, bcc = 0;
  Control control;

  //printf("Bytes read: \n");

  while(resend == FALSE) {

    if(state != STOP)
      read(fd, &ch, 1);                

    switch(state) {
      case START:
        if (ch == FLAG){
          bcc = 0;
          state = FLAG_RCV;
        }
        break;

      case FLAG_RCV:
        if (ch == expectedAddress(period,status)){
          state = A_RCV;
          bcc = createBCC(bcc,ch);
        }
        else if (ch != FLAG)
          state = START;
        break;

      case A_RCV:
        if (expectedControl(period, status, ch) == TRUE){
          control = ch;
          state = C_RCV;
          bcc = createBCC(bcc,ch);
        }
        else if (ch != FLAG)
          state = START;
        else
          state = FLAG_RCV;
        break;

      case C_RCV:
        if (ch == FLAG)
          state = FLAG_RCV;
        else if (ch == bcc)
          state = BCC_OK;
        else
          state = START;  
        break;

      case BCC_OK:
        if(ch == FLAG) 
          state = STOP;
        break;

      case STOP:
        printf("LinkLayer: Received %s message \n", getControlName(control));
        return control;
    }
  }
  
  return -1;

}


Control expectedAddress(Period period, Status status){
  if((period == DISCONNECT && status == TRANSMITTER) || (period == END && status == RECEIVER))
    return A_END;
  return A;
}


int expectedControl(Period period, Status status, unsigned char ch) {
  int result = FALSE;

  switch (status)
  {
    case RECEIVER:
      if((period == SETUP && ch == C_SET)|| // Estabelecimento => Recetor espera SET
        (period == DISCONNECT && ch == C_DISC)|| // Terminação => Recetor espera DISC
        (period == END && ch == C_UA))  // Terminação(acknowlegment) => Recetor espera UA
        result = TRUE;

      break;

    case TRANSMITTER: 
      if((period == SETUP && ch == C_UA)|| // Estabelecimento => Transmissor espera UA
        (period == DISCONNECT && ch == C_DISC)) // Terminação => Recetor espera DISC
        result = TRUE;

      else if(period == TRANSFER){  // Transferencia => Transmissor espera RR ou REJ
        if(((ch == C_RR_1 || ch == C_REJ_0) && dataLink->sequenceNumber == 0)|| // Se ns = 0, espera RR_1 ou REJ_0
          ((ch == C_RR_0 || ch == C_REJ_1) && dataLink->sequenceNumber == 1))  // Se ns = 1, espera RR_0 ou REJ_1
          result = TRUE;
      }

      break;

    default:
      perror("Invalid Status");
      break;
  }

  return result;
}


int sendSupervisionFrame(int fd, Control control, Status status) {
  unsigned char buf[5];
  int nw;
  
  buf[0] = FLAG;

  if((control == C_DISC && status == RECEIVER) || (control == C_UA && status == TRANSMITTER))
    buf[1] = A_END;
  else
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

  printf("LinkLayer: Sent %s message\n", getControlName(control));
  return 0;

}


// Tranferencia de Dados - leitura da porta de serie

int llread(int fd, unsigned char* buffer){

  int dataFieldSize = receiveInfoFrame(fd); // Recebe trama de informação
  int validDataField = TRUE;
  
  if(dataFieldSize == -1) 
    validDataField = FALSE;
  else if(dataFieldSize == 0 && dataLink->frame[CONTROL_BYTE] == C_DISC)
    return -1;


  // Verificar se o ns recebido é o que se pretende
  int expectedSequenceNumber = FALSE;
  int receivedSequenceNumber = (dataLink->frame[CONTROL_BYTE] >> 6) & 0x01;
  if(receivedSequenceNumber == dataLink->sequenceNumber)
    expectedSequenceNumber = TRUE;
  

  Control ack = buildAck(validDataField, expectedSequenceNumber);

  if(sendSupervisionFrame(fd, ack, RECEIVER) == -1 ){
    fprintf(stderr,"Error sending %s\n", getControlName(ack));
    restoreConfiguration(fd,&oldtio);
    free(dataLink);
    return -1;
  }

  if(validDataField == TRUE && expectedSequenceNumber == TRUE)
    memcpy(buffer,&dataLink->frame[HEADER_SIZE], dataFieldSize);
  else
    dataFieldSize = 0;
  
    
  return dataFieldSize;
}


int receiveInfoFrame(int fd) {

  State iState = START;
  unsigned char ch, bcc1 = 0;
  int nr, i = 0; 
  int end = FALSE; 

  //printf("Bytes read: \n");

  while(!end && i< MAX_INFO_FRAME) {

    if(iState != STOP){
      nr = read(fd, &ch, 1);
      /*if(nr > 0)
        printf("%4X",ch);*/
    }                  

    switch(iState) {
      case START:
        if (ch == FLAG){
          bcc1 = 0;
          i = 0;
          memset(dataLink->frame, 0, MAX_INFO_FRAME); 

          iState = FLAG_RCV;
          dataLink->frame[i] = ch;
          i++;
        }
        break;

      case FLAG_RCV:
        if (ch == A){
          bcc1 = createBCC(bcc1,ch);

          iState = A_RCV;
          dataLink->frame[i] = ch;
          i++;
        }
        else if (ch != FLAG)
          iState = START;
        break;

      case A_RCV:
        if (isInfoSequenceNumber(ch) == TRUE){
          bcc1 = bcc1 ^ch;

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
        else{
          iState = START; 
        } 
        break;

      case BCC_OK:
        if(nr > 0){
          dataLink->frame[i] = ch;
          i++;
        }
        if(ch == FLAG){
          iState = STOP;
        }
        break;

      case STOP:
        printf("LinkLayer: Received Information Frame with NS = %d\n",dataLink->frame[CONTROL_BYTE]>>6 & 0x01);
        end = TRUE;
    
    }
  }

  if(i == MAX_INFO_FRAME && end == FALSE){
    printf("max I frame size exceeded");
    return -1;
  }

  if(ch == C_DISC)
    return 0;
  
  i = byteDestuffing(i);

  int dataSize = i - DELIMIT_INFO_SIZE;
  if(validBcc2(&dataLink->frame[HEADER_SIZE], dataSize + 1) != -1)
    return dataSize;

  return -1; // Erro no BCC2

}


Control buildAck(int validDataField, int expectedSequenceNumber){

  if(validDataField == TRUE){ // Tramas sem erros nos dados
    if(expectedSequenceNumber == TRUE) // Nova Trama
      dataLink->sequenceNumber = !dataLink->sequenceNumber; // Passa a pedir o outro Numero de Sequencia  
     
    if(dataLink->sequenceNumber == 0)
      return C_RR_0;
    else
      return C_RR_1;
  }

  // Tramas com erros nos dados
  if(expectedSequenceNumber == TRUE){ // Nova Trama - pedido de retransmissao
    if(dataLink->sequenceNumber == 0)
      return C_REJ_0;
    return C_REJ_1;
  }

  // Trama repetida com erros nos dados
  if(dataLink->sequenceNumber == 0)
    return C_RR_0;
  return C_RR_1;

}


// Tranferencia de Dados - escrita na porta de serie

int llwrite(int fd, unsigned char* buffer, int length) {

  Control controlByte;

  if(dataLink->sequenceNumber == 0)
    controlByte = C_N0;
  else controlByte = C_N1;


  if (createFrameI(controlByte, buffer, length) != 0) {
    restoreConfiguration(fd, &oldtio);
    free(dataLink);
    return -1;
  }

  //stuffing
  int frameSize; //length after stuffing
  if((frameSize = byteStuffing(length)) < 0){
    restoreConfiguration(fd, &oldtio);
    free(dataLink);
    return -1;
  }

  int numWritten;
  int receivedControl;

  tries = 0;     
  resend = FALSE; 
  
  // Mecanismo de retransmissão da trama I
  while(tries < dataLink->numTransmissions){
    
    if((numWritten = sendFrameI(fd, frameSize)) == -1) {
      restoreConfiguration(fd, &oldtio);
      free(dataLink);
      return -1;
    }

    resend = FALSE; 
    alarm(dataLink->timeout); // Inicia espera por RR ou REJ 

    // Recebeu RR ou REJ esperados (se enviou I0 recebeu RR_1 ou REJ_0 | se enviou I1 recebeu RR_0 ou REJ_1)
    if ((receivedControl = receiveSupervisionFrame(fd, TRANSFER, TRANSMITTER)) != -1){

      if(receivedControl >> 7 == !dataLink->sequenceNumber){  // Eviou I0 e recebeu RR_1 | Enviou I1 e recebeu RR_0
        resend = FALSE; 
        alarm(0); // Desativa alarme
        tries = 0;
        dataLink->sequenceNumber = !dataLink->sequenceNumber; // Altera numero de serie para a proxima trama
        break;
      }
      else{ // enviou I0 e recebeu REJ_0 | enviou I1 e recebeu REJ_1
        printf("LinkLayer: Received REJ\n");
        resend = FALSE; 
        alarm(0); // Desativa alarme
        tries++;
      }

    } 
    printf("LinkLayer: Retransmit Data Packet\n");
    // Ocorreu timeout ou alarme foi antecipado por REJ => retransmitir
  }

  if(tries == dataLink->numTransmissions){
    printf("LinkLayer:  Retransmission attempts to send Data Packet and receive RR exceeded\n");
    return -1;
  }

  return (numWritten - DELIMIT_INFO_SIZE); // length of the data packet length sent to the receiver
}


int createFrameI(Control controlField, unsigned char* infoField, int infoFieldLength) {

  memset(dataLink->frame, 0, MAX_INFO_FRAME);
  
  dataLink->frame[0] = FLAG;

  dataLink->frame[1] = A; 

  dataLink->frame[2] = controlField;

  dataLink->frame[3] = dataLink->frame[1] ^ dataLink->frame[2];

  for(int i = 0; i < infoFieldLength; i++) {
    dataLink->frame[i + 4] = infoField[i];
  }

  unsigned bcc2 = createBCC_2(infoField, infoFieldLength);

  dataLink->frame[infoFieldLength + 4] = bcc2;

  dataLink->frame[infoFieldLength + 5] = FLAG;

  return 0;
}


int sendFrameI(int fd, int length) {
    tcflush(fd, TCIOFLUSH);

    int n;
    if((n = write(fd, dataLink->frame, length)) <= 0){
      return -1;
    }
    return n;
}


// Byte Stuffing e Destuffing

int byteStuffing(int infoFieldLength) {

  int frameSize = infoFieldLength + DELIMIT_INFO_SIZE;
  unsigned char *aux = malloc(sizeof(unsigned char) * frameSize);  // buffer aux
  if(aux == NULL){
    return -1;
  }
  
  for(int i = 0; i < frameSize ; i++){ // Copy frame to aux
    aux[i] = dataLink->frame[i];
  }

  int frameIdx = HEADER_SIZE; // Starts with 1st data Idx
  int lastFlagIdx = frameSize - 1;

  for(int auxIdx = HEADER_SIZE; auxIdx < frameSize; auxIdx++){ //fills frame buffer

    if(aux[auxIdx] == FLAG && auxIdx != lastFlagIdx) { 
      dataLink->frame[frameIdx] = ESC;
      dataLink->frame[frameIdx + 1] = STUFFING_FLAG;
      frameIdx += 2;
    }
    else if(aux[auxIdx] == ESC) {
      dataLink->frame[frameIdx] = ESC;
      dataLink->frame[frameIdx + 1] = STUFFING_ESC;
      frameIdx += 2;
    }
    else{
      dataLink->frame[frameIdx] = aux[auxIdx];
      frameIdx++;
    }
  }

  printf("LinkLayer:  Stuffing complete.\n");
  /*for(int i = 0; i < frameIdx; i++){
    printf("%4X",dataLink->frame[i]);
  }
  printf("\n");*/

  free(aux);
  return frameIdx;
}


int byteDestuffing(int length){
 
  for(int i = HEADER_SIZE; i < length; i++){
    if(dataLink->frame[i] == ESC){
      memmove(&dataLink->frame[i], &dataLink->frame[i+1], length-i-1);
      dataLink->frame[i] ^= STUFF_OCT;
      length--;
    }
  }

  printf("LinkLayer:  Destuffing complete\n");
  /*for(int i = 0; i < length; i++){
    printf("%4X",dataLink->frame[i]);
  }
  printf("\n");*/

  return length;
}


// Terminacao da ligacao de dados

int llclose(int fd,  Status status){
  printf("\nLinkLayer: llclose\n\n");
  tries = 0;     
  resend = FALSE; 

  switch (status)
  {
    case RECEIVER:
      if(closeReceiver(fd) == -1){
        restoreConfiguration(fd, &oldtio);
        free(dataLink);
        return -1;
      }
      break;
    
    case TRANSMITTER:
      if(closeTransmitter(fd) == -1){
        restoreConfiguration(fd, &oldtio);
        free(dataLink);
        return -1;
      }
      break;
      
    default:
      perror("Invalid Status: nor RECEIVER nor TRANSMITTER");
      restoreConfiguration(fd, &oldtio);
      free(dataLink);
      return -1;
  }


  restoreConfiguration(fd,&oldtio);
  free(dataLink);

  return 0;
}


int closeReceiver(int fd){
    
  // Recebe da trama DISC
  receiveSupervisionFrame(fd, DISCONNECT, RECEIVER);
  
  // (Re)transmissao da trama DISC 
  while(tries < dataLink->numTransmissions){
    
    if(sendSupervisionFrame(fd, C_DISC, RECEIVER) == -1){ // Envia DISC
      perror("Error sending DISC");
      return -1;
    }

    resend = FALSE; 
    alarm(dataLink->timeout); // Inicia espera por UA

    if (receiveSupervisionFrame(fd, END, RECEIVER) != -1){ // Espera UA
      resend = FALSE; 
      alarm(0);
      tries = 0;
      return 0;
    } 
    // Se não receber UA volta a enviar DISC - retransmissão
    printf("LinkLayer: Retransmit Disc\n");
  }

  printf("LinkLayer: Retransmission attempts to send DISC and receive UA exceeded\n");
  return -1;

}


int closeTransmitter(int fd){

  // (Re)transmissao da trama DISC
  while(tries < dataLink->numTransmissions){
    
    if(sendSupervisionFrame(fd, C_DISC, TRANSMITTER) == -1){
      perror("Error sending DISC");
      return -1;
    }

    resend = FALSE; 
    alarm(dataLink->timeout); // Inicia espera por DISC

    if (receiveSupervisionFrame(fd, DISCONNECT, TRANSMITTER) != -1){
      resend = FALSE; 
      tries = 0;
      alarm(0);
      break;
    } 
    // Se não receber DISC volta a enviar DISC - retransmissão
    printf("LinkLayer: Retransmit Disc\n");
  }

  if(tries == dataLink->numTransmissions){
    printf("LinkLayer: Retransmission attempts to send DISC and receive DISC exceeded\n");
    return -1;
  }

  if(sendSupervisionFrame(fd, C_UA, TRANSMITTER) == -1){
    perror("Error sending UA");
    return -1;
  }

  return 0;
}