#ifndef MACROS_HEADER
#define MACROS_HEADER

#define BAUDRATE          B38400
#define _POSIX_SOURCE     1    /* POSIX compliant source */

#define FALSE   0
#define TRUE    1

#define TIMEOUT           3   
#define MAX_RETR          3   

// APPLICATION LAYER

#define DATA_SIZE          255
#define PACKET_HEADER      4
#define PACKET_SIZE        DATA_SIZE + PACKET_HEADER

#define MAX_FILE           255

#define PACKET_L1   2
#define PACKET_V1   3

// Error Generation
#define T_PROP             200.0   // Tempo de propagação em ms
#define BCC1_ERROR         0.0   // Probabilidade de surgir erro no BCC1
#define BCC2_ERROR         0.0   // Probabilidade de surgir erro no BCC2

// Tipos de pacotes ao nível da Aplicação
typedef enum PacketControl {
    CTRL_PACKET_DATA = 0x01,
    CTRL_PACKET_START = 0x02,
    CTRL_PACKET_END = 0x03
} PacketControl;

typedef enum PacketParam {
    FILESIZE = 0x00,
    FILENAME = 0x01
} PacketParam;


// DATA LINK LAYER

#define HEADER_SIZE        4
#define DELIMIT_INFO_SIZE  6
#define CONTROL_BYTE      2

#define MAX_DATA_FRAME     PACKET_SIZE + DELIMIT_INFO_SIZE
#define MAX_INFO_FRAME     ((MAX_DATA_FRAME + 1) * 2) + HEADER_SIZE + 1


/* DATA LINK */

typedef enum Status {
    RECEIVER, TRANSMITTER
} Status;

#define A               0x03         /* Campo de Endereço em Respostas enviadas pelo Receptor e Comandos enviados pelo Emissor */
#define A_END           0x01         /* Campo de Endereço em Respostas enviadas pelo Emissor e Comandos enviados pelo Recetor */
#define FLAG            0x7E         /* Flag que delimita as tramas */
#define STUFFING_FLAG   0x5E
#define STUFFING_ESC    0x5D
#define ESC             0x7D         /* Octeto de escape */
#define STUFF_OCT       0x20

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
    C_N0 = 0x00,
    C_N1 = 0x40,
    C_DISC = 0x0B
} Control;

typedef enum Period {
    SETUP,              /* Estabelecimento da ligação de dados (SET, UA) */
    TRANSFER,           /* Transferência de dados (I, RR, REJ)*/
    DISCONNECT,         /* Terminação (DISC, UA)*/
    END                 /* Estabelecimento da ligação de dados (UA)*/
} Period;

#endif
