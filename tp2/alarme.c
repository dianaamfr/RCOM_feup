#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#define TIMEOUT 3
int flag=1, conta=1;

void atende()                   // atende alarme
{
	printf("alarme # %d\n", conta);
	flag=1;
	conta++;
}


main()
{

(void) signal(SIGALRM, atende);  // instala  rotina que atende interrupcao

while(conta < 4){
   if(flag){
      alarm(TIMEOUT);                 // activa alarme de 3s
      flag=0;
   }
}
printf("Vou terminar.\n");

}

