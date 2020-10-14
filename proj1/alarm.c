
#include <stdio.h>
#include "alarm.h"

void alarmHandler(int sig){
    
    resend = TRUE; /* Alarme foi accionado => Reenviar trama */
    tries++;

    printf("\nTimeout! Tries used: %d \n", tries);
    if(tries == MAX_RETR)
        printf("Maximum retransmission tries exceeded.\n");

    return;
}