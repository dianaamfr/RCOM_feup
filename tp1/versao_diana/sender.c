/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define BUFSIZE 255

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res, n;
    struct termios oldtio,newtio;
    char buf[BUFSIZE], reply[BUFSIZE];
    int i, sum = 0, speed = 0;
    
    /* Check Serial Port Argument */
    if ( (argc < 2) || ((strcmp("/dev/ttyS0", argv[1])!=0) &&
                        (strcmp("/dev/ttyS1", argv[1])!=0))) {
      printf("Usage: %s /dev/ttySx\n\tex: %s /dev/ttyS1\n",argv[0], argv[0]);
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd < 0) {perror(argv[1]); exit(-1); }

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

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 char received */

  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

  
    printf("Write your message: ");
    /* Read a line from stdin - max 255;
       fgets gets until 254 characters + '\0'
    */
    if(fgets(buf,BUFSIZE,stdin) == NULL){
      perror("fgets");
      exit(-1);
    }; 

    n = strlen(buf); /* Count number of characters written (including '\n') */
    buf[n-1] = '\0'; /* Adds null terminator to the message (excludes new line) */
    
    res = write(fd,buf,n);  /* Write n characters -  includes ‘\0’ to indicate the end of the transmission to the receptor */ 
    printf("%d bytes written\n", res); /* Print number of bytes written */

    int j = 0;
    res = 0;
    while(1){
      read(fd, &reply[j], 1); /* Read the message resent by the receiver*/
      res = read(fd,&reply[i],1);     /* returns after 1 char/byte has been input */
      if (reply[j] == '\0'){
        break;
      }
      j++;
    }

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
    return 0;
}
