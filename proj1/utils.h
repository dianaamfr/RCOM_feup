#ifndef UTILS_HEADER
#define UTILS_HEADER

/*
 * Abetura do descritor da porta de série em modo não canónico
 * @param port porta de serie
 * @param oldtio configuração anterior da porta série
 * @return descritor da porta de serie
*/
int openNonCanonical(char* port, struct termios* oldtio);

int restoreConfiguration(int fd, struct termios* oldtio);

#endif
