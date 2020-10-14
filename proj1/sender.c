#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "datalink.h"
#include "alarm.h"
#include "utils.h"

 extern unsigned int tries;     
 extern unsigned int resend; 


int main(int argc, char** argv) {

  signal(SIGALRM,alarmHandler); // Instala rotina que atende interrupcao do alarme
    
  /*if ( (argc < 2) || 
        ((strcmp("/dev/ttyS0", argv[1])!=0) && 
        (strcmp("/dev/ttyS1", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }*/

  struct termios oldtio;

  int fd = openNonCanonical(argv[1],&oldtio);

  tries = 0;     
  resend = FALSE; 

  while(tries < MAX_RETR){ /* Enquanto nao se tiverem esgotado as tentativas */
    
    sendControl(fd, C_SET); /* Transmite/Retransmite trama SET */

    resend = FALSE; /* A Flag passa a indicar que o tempo ainda nao se esgotou */
    alarm(TIMEOUT); /* Ativacao do alarme para esperar por UA válida */

    if (receiveControl(fd, C_UA) == TRUE){ /* Ao receber a trama UA desativa o alarme e termina */
      alarm(0);
      break;
    } 

  }
  
  restoreConfiguration(fd, &oldtio);
  
  return 0;
}
