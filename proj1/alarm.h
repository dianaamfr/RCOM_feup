#ifndef ALARM_HEADER
#define ALARM_HEADER

#include "macros.h"

unsigned int tries ;     /* Numero de tentativas de envio das tramas - MAXRETR + tentativa inicial */
unsigned int resend; /* Flag para assinalar quando passou o TIMEOUT e é necessário reenviar a trama*/
unsigned int maxTries;

/**
 * Handler para o sinal SIGALARM
 * 
 * Assinala que o alarme foi acionado através - flag resend;e incrementa o numero de tentativas usadas
 * 
 * @param sig sinal recebido
*/
void alarmHandler(int sig);

/**
 * Define o número de tentativas usadas para transmissão de tramas
 * 
 * @param int max
*/
void setMaxTries(unsigned int max);

#endif
