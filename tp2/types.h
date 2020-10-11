/* Estado da rececao da trama SET */
typedef enum setRcvState {
    START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, OTHER_RCV, STOP
} setRcvState;
