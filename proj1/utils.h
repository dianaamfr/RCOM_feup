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

#endif
