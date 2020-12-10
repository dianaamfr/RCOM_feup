
#include <stdio.h>
#include "alarm.h"

void alarmHandler(int sig){
    
    resend = TRUE; /* Alarme foi accionado => Reenviar trama */
    tries++;

    return;
}

void setMaxTries(unsigned int max){
    maxTries = max;
}
