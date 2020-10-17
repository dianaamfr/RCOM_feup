#ifndef MACROS_HEADER
#define MACROS_HEADER

#define BAUDRATE          B38400
#define _POSIX_SOURCE     1    /* POSIX compliant source */

#define FALSE   0
#define TRUE    1

#define TIMEOUT           3   
#define MAX_RETR          3   

#define HEADER_SIZE       4
#define MAX_DATA_FIELD    1024
#define MAX_INFO_FRAME    MAX_DATA_FIELD + 6

#define CONTROL_BYTE      2

/* DATA LINK */

typedef enum Status {
    RECEIVER, TRANSMITTER
} Status;

#define A       0x03         /* Campo de Endereço em Respostas enviadas pelo Receptor */
#define FLAG    0x7E         /* Flag que delimita as tramas */

/* Estado da rececao da trama SET */
typedef enum State {
    START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP
} State;


typedef enum Control {
    C_UA = 0x07,
    C_SET = 0x03,
    C_RR_0 = 0x05,
    C_RR_1 = 0x85,
    C_REJ_0 = 0x01,
    C_REJ_1 = 0x81,
    I_0 = 0x00,
    I_1 = 0x40
} Control;

#endif
