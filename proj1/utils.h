#ifndef UTILS_HEADER
#define UTILS_HEADER

#include "macros.h"

/**
 * Valida os argumentos - verifica se é uma porta de série válida
 * @param argc número de argumentos
 * @param argv argumentos
 * @return 0 no caso de a porta ser válida, -1 caso contrário
*/
int validateArgs(int argc, char** argv);

/**
 * Abetura do descritor da porta de série em modo não canónico
 * @param port porta de serie
 * @param oldtio configuração anterior da porta série
 * @return descritor da porta de serie
*/
int openNonCanonical(char* port, struct termios* oldtio);

/**
 * Restaura configuração inicial da porta de série
 * @param port porta de serie
 * @param oldtio configuração anterior da porta série
 * @return 0 no caso de sucesso e -1 no caso de insucesso
*/
int restoreConfiguration(int fd, struct termios* oldtio);

/**
 * Converte valor do campo de controlo(enum) numa string
 * @param control o campo de controlo(enum) a converter
 * @return nome do campo de controlo em string
*/
char* getControlName(Control control);

/**
 * Retorna o XOR (ou exclusivo) dos octetos a e c
 * @param a octeto 
 * @param c octeto
 * @return ou exclusivo dos octetos
*/
unsigned char createBCC(unsigned char a, unsigned char c);

/**
 * Retorna o XOR (ou exclusivo) dos octetos da frame
 * @param frame campo de dados
 * @param length tamanho do campo de dados
 * @return ou exclusivo dos octetos
*/
unsigned char createBCC_2(unsigned char* frame, int length);

/**
 * Validação do BCC2 
 * @param dataField campo de dados(bcc2 inclusive)
 * @param length tamanho do campo de dados
 * @return 0 se válido e -1 se inválido
*/
int validBcc2(unsigned char * dataField, int length);

/**
 * Verifica se o campo de controlo correponde ao de uma trama de informação
 * @param byte o octeto a verificar
 * @return 0(TRUE) no caso de se tratar de uma trama de informação e 1(FALSE) caso contrário
*/
int isInfoSequenceNumber(unsigned char byte);

#endif
