#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include "datalink.h"
#include "alarm.h"
#include "utils.h"


int llopen(int porta, connectionRole role){
    return 0;
}

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
  if (nw != sizeof(buf))
		perror("Error writing UA\n");

  printf("Sent %s message with success\n", getControlName(control));

  return 0;

}