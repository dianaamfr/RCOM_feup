#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"

int validateArgs(int argc, char** argv) {
 
    if((argc < 2) ||
        ((strcmp("/dev/ttyS0", argv[1])!=0) &&
        (strcmp("/dev/ttyS1", argv[1])!=0) &&
        (strcmp("/dev/ttyS10", argv[1])!=0) &&
        (strcmp("/dev/ttyS11", argv[1])!=0)))
        return -1;
    
    return 0;
}


int openNonCanonical(char* port, struct termios* oldtio){

    int fd;
    struct termios newtio;
    fd = open(port, O_RDWR | O_NOCTTY );

    if (fd < 0) {
        perror(port);
        exit(-1);
    }

    if (tcgetattr(fd, oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    newtio.c_lflag = 0; /* set input mode (non-canonical, no echo,...) */

    /* 
        VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
        leitura do(s) proximo(s) caracter(es).

        VTIME = 0 & VMIN = 0 read will be satisfied immediately. The number of characters currently
        available, or the number of characters requested will be returned.
    */
    newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
    newtio.c_cc[VMIN] = 0;  /* sets the minimum number of characters to receive before satisfying the read. */

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    return fd;

}


int restoreConfiguration(int fd, struct termios* oldtio){
    tcsetattr(fd,TCSANOW, oldtio);
    return close(fd);
}


char* getControlName(Control control) {
    char * str = "";

    switch (control)
    {
    case C_SET:
        str = "SET";
        break;

    case C_UA:
        str ="UA";
        break;

    case C_RR_0:
        str ="RR_0";
        break;

    case C_RR_1:
        str ="RR_1";
        break;

    case C_REJ_0:
        str ="REJ_0";
        break;

    case C_REJ_1:
        str ="REJ_1";
        break;
    
    case I_0:
        str ="I_0";
        break;

    case I_1:
        str ="I_1";
        break;
    
    default:
        break;
    }

    return str;
};
