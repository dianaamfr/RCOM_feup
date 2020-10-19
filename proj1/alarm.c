
#include <stdio.h>
#include "alarm.h"

void alarmHandler(int sig){
    
    resend = TRUE; /* Alarme foi accionado => Reenviar trama */
    tries++;

    printf("\nTimeout! Tries used: %d \n", tries);
    if(tries == maxTries)
        printf("Maximum retransmission tries exceeded.\n");

    return;
}

void setMaxTries(unsigned int max){
    maxTries = max;
}
