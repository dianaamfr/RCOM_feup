#ifndef MACROS
#define MACROS

#define BAUDRATE          B38400
#define _POSIX_SOURCE     1    /* POSIX compliant source */
#define FALSE   0
#define TRUE    1

#define TIMEOUT           3   
#define MAX_RETR          3   

/* DATA LINK */

#define A       0x03         /* Campo de Endereço em Respostas enviadas pelo Receptor */
#define FLAG    0x7E         /* Flag que delimita as tramas */

/* Estado da rececao da trama SET */
typedef enum state {
    START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP
} state;


typedef enum control {
    C_UA = 0x07,
    C_SET = 0x03
} control;

#endif
