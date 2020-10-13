#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <utils.h>

#define TIMEOUT 3
#define TRIES 3
int flag = 1;
int conta = 0;

void atende()                   // atende alarme
{
	printf("alarme # %d\n", conta);
	flag = 1;
	conta++;
}


main()
{

(void) signal(SIGALRM, atende);  // instala  rotina que atende interrupcao

while(conta < TRIES){
   if(flag){
      alarm(TIMEOUT);                 // activa alarme de 3s
      flag=0;
   }
}
printf("Vou terminar.\n");

}

