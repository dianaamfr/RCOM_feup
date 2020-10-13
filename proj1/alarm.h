#ifndef ALARM_HEADER
#define ALARM_HEADER

#include "macros.h"

unsigned int tries ;     /* Numero de tentativas usadas para retransmissao do comando SET e espera do UA */
unsigned int resend; /* Flag para assinalar quando passou o TIMEOUT e é necessário reenviar a trama*/

/**
 * Handler para o sinal SIGALARM
 * 
 * Assinala que o alarme foi acionado através - flag resend;e incrementa o numero de tentativas usadas
 * 
 * @param sig sinal recebido
*/
void alarmHandler(int sig);

#endif
