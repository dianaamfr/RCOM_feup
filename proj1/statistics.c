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

    errorBcc1 = 0;
    errorBcc2 = 0;

   return 0;
}


void saveFileInfo(char * fileName, int fileSize){
    fprintf(statsPtr, "File size: %d bytes\n", fileSize);
    fprintf(statsPtr, "File name: %s \n", fileName);
}


void saveStats(struct timeval * begin, int bits){

    struct timeval end;
    gettimeofday(&end, 0);

    long seconds = end.tv_sec - begin->tv_sec;
    long microseconds = end.tv_usec - begin->tv_usec;

    double elapsed = seconds + microseconds*1e-6;
    fprintf(statsPtr, "Tempo Total: %.4f s\n", elapsed);

    double R = bits * 8.0 / elapsed;
    fprintf(statsPtr, "Debito recebido (R): %.4f bits/s\n", R);

    double S = R / get_speed();
    fprintf(statsPtr, "Eficiencia (S): %.4f\n", S);

    fprintf(statsPtr, "Baudrate: %s\n", see_speed(BAUDRATE));

    fprintf(statsPtr, "Tamanho dos Blocos: %d\n", DATA_SIZE);

    fprintf(statsPtr, "T prop: %f\n", T_PROP);

    fprintf(statsPtr, "P(erro no Bcc1): %f   P(erro no Bcc2): %f\n", BCC1_ERROR , BCC2_ERROR);

    fprintf(statsPtr, "Erros Bcc1: %d   Erros Bcc2: %d\n", errorBcc1 , errorBcc2);

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


double get_speed() {
  double r;
  switch (BAUDRATE) {
    case B0:       r = 0;
                   break;
    case B50:      r = 50;
                   break;
    case B75:      r = 75;
                   break;
    case B110:     r = 110;
                   break;
    case B134:     r = 134;
                   break;
    case B150:     r = 150;
                   break;
    case B200:     r=200;
                   break;
    case B300:     r = 300;
                   break;
    case B600:     r = 600;
                   break;
    case B1200:    r = 1200;
                   break;
    case B1800:    r = 1800;
                   break;
    case B2400:    r = 2400;
                   break;
    case B4800:    r = 4800;
                   break;
    case B9600:    r = 9600;
                   break;
    case B19200:   r = 19200;
                   break;
    case B38400:   r = 38400;
                   break;
    default:       r = 38400;
  }
  return r;
}


int generateBCC1Error(){
    double r = (double)rand() / RAND_MAX;

    if(r < BCC1_ERROR){
        errorBcc1++;
        return TRUE;
    }

    return FALSE;
}


int generateBCC2Error(){

    double r = (double)rand() / RAND_MAX;

    if(r < BCC2_ERROR){
        errorBcc2++;
        return TRUE;
    }

    return FALSE;
}


int endStats(){
    
    fprintf(statsPtr, "\n");

    if(fclose(statsPtr) < 0){
        perror("Error closing statistics file");            
        return -1;
    }
    return 0;
}
