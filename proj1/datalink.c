#include <stdio.h>
#include <unistd.h>
#include "datalink.h"
#include "alarm.h"


int llopen(int porta, connectionRole role){
    return 0;
}

int receiveControl(int fd, control C) {

  state uaState = START;
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
        if (ch == C){
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
        printf("\nReceived UA message with success\n");
        return TRUE;
    }
  }
  
  return FALSE;

}
