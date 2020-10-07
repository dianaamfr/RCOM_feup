/* Campo de Controlo */
typedef enum control{
	C_SET = 0x03,           /* set up */
    C_DISC = 0x0B,          /* disconnect */
    C_UA = 0x07,            /* unnumbered acknowledgment */
    C_RR = 0x05,            /* receiver ready / positive ACK; 1º bit = numero de sequencia */
    C_REJ = 0x01            /* reject / negative ACK; 1º bit = numero de sequencia */
} control;


/* Estado da rececao da trama SET */
typedef enum setRcvState {
    START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, OTHER_RCV, STOP
} setRcvState;


#define FLAG  0x7E        /* Flag que delimita as tramas */


/* 
  I, SET e DISC => Comandos
  UA, RR, REJ => Respostas
*/
typedef enum address {
    A_CMD_E = 0x03,     /* Campo de Endereço em Comandos enviados pelo Emissor */
    A_ANS_R = 0x03,     /* Campo de Endereço em Respostas enviadas pelo Receptor */
    A_CMD_R = 0x01,     /* Campo de Endereço em Comandos enviados pelo Receptor */
    A_ANS_E = 0x01      /* Campo de Endereço em Respostas enviadas pelo Emissor */
} address;
