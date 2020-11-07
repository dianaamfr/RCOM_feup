#ifndef STATISTICS_HEADER
#define STATISTICS_HEADER

#include <stdio.h>
#include <termios.h> 

FILE * statsPtr;

/* Para registar estatísticas */

int registerStats();

void saveFileInfo(char * fileName, int fileSize);

void saveStats(struct timeval * begin, int bits);

char *see_speed(speed_t speed);

double get_speed();

int endStats();


/* Para simular erros */

int generateBCC1Error();

int generateBCC2Error();

#endif