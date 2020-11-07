#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include "statistics.h"
#include "macros.h"


int registerStats(){
    if ((statsPtr = fopen("stats.txt","a")) == NULL){
        perror("Error opening stats file!");
        return 1;
    }

   return 0;
}


void saveFileInfo(char * fileName, int fileSize){
    fprintf(statsPtr, "File size: %d bits\n", fileSize);
    fprintf(statsPtr, "File name: %s \n", fileName);
}


void saveStats(struct timeval * begin, int bits){

    struct timeval end;
    gettimeofday(&end, 0);

    long seconds = end.tv_sec - begin->tv_sec;
    long microseconds = end.tv_usec - begin->tv_usec;

    double elapsed = seconds + microseconds*1e-6;
    fprintf(statsPtr, "Tempo Total: %.4f seconds.\n", elapsed);

    double R = bits * 8.0 / elapsed;
    fprintf(statsPtr, "Debito recebido (R): %.4f bits/s\n", R);

    double S = R / BAUDRATE;
    fprintf(statsPtr, "Eficiencia (S): %.4f.\n", S);

    fprintf(statsPtr, "Baudrate: %s\n", see_speed(BAUDRATE));

    fprintf(statsPtr, "Block Size: %d\n", DATA_SIZE);

    fprintf(statsPtr, "T prop: %f\n", T_PROP);

    fprintf(statsPtr, "Bcc1 Error: %f   Bcc2 Error: %f\n", BCC1_ERROR , BCC2_ERROR);

    endStats();
}


char *see_speed(speed_t speed) {
  static char SPEED[20];
  switch (speed) {
    case B0:       strcpy(SPEED, "B0");
                   break;
    case B50:      strcpy(SPEED, "B50");
                   break;
    case B75:      strcpy(SPEED, "B75");
                   break;
    case B110:     strcpy(SPEED, "B110");
                   break;
    case B134:     strcpy(SPEED, "B134");
                   break;
    case B150:     strcpy(SPEED, "B150");
                   break;
    case B200:     strcpy(SPEED, "B200");
                   break;
    case B300:     strcpy(SPEED, "B300");
                   break;
    case B600:     strcpy(SPEED, "B600");
                   break;
    case B1200:    strcpy(SPEED, "B1200");
                   break;
    case B1800:    strcpy(SPEED, "B1800");
                   break;
    case B2400:    strcpy(SPEED, "B2400");
                   break;
    case B4800:    strcpy(SPEED, "B4800");
                   break;
    case B9600:    strcpy(SPEED, "B9600");
                   break;
    case B19200:   strcpy(SPEED, "B19200");
                   break;
    case B38400:   strcpy(SPEED, "B38400");
                   break;
    default:       sprintf(SPEED, "unknown (%d)", (int) speed);
  }
  return SPEED;
}


int generateBCC1Error(){
    double r = (double)rand() / RAND_MAX;
    return r < BCC1_ERROR;
}


int generateBCC2Error(){
    double r = (double)rand() / RAND_MAX;
    return r < BCC2_ERROR;
}


int endStats(){
    
    fprintf(statsPtr, "\n");

    if(fclose(statsPtr) < 0){
        perror("Error closing statistics file");            
        return -1;
    }
    return 0;
}
