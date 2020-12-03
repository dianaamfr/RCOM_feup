/*      (C)2000 FEUP  */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <strings.h>

/* Atenção - a implementar slide 36 mas só a parte do cliente */

#define SERVER_PORT 6000 /* #define FTP_PORT_NUMBER 21  => FILE TRANSFER PROTOCOL PORT */ 
// Aqui já é dado o ip mas nós vamos receber o host name e temos que ir buscar o ip com gethostbyname
#define SERVER_ADDR "192.168.28.96"

int main(int argc, char** argv){

	// TODO - Processar argumentos. "ftp://[<user>:<password>@]<host>/<url-path>"
	// Usar anonymous se não for indicado
	
	char	buf[] = "Mensagem de teste na travessia da pilha TCP/IP\n";  
	int	bytes;

	/* TODO - usar gethostbyname e mais funções necessárias para obter o IP a partir do Host Name 
	
	-slide 38 tem coisas mal; Ver man gethostbyname, https://www.gta.ufrj.br/ensino/eel878/sockets/gethostbynameman.html e getip.c
	-struct hostent *he;
	-gethostbyname() takes a string like "www.yahoo.com", and returns a struct hostent 
	which contains tons of information, including the IP address.

	Alguns campos da struct hostent:
	- char **h_addr_list => list of IP addresses for this host. 
	Although this is a char**, it's really an array of struct in_addr*s in disguise. The last array element is NULL.

	-h_addr A commonly defined alias for h_addr_list[0]. 
	If you just want any old IP address for this host (yeah, they can have more than one) just use this field.

	Ex:

	int i;
		struct hostent *he;
		struct in_addr **addr_list;
		struct in_addr addr;

		// get the addresses of www.yahoo.com:   
		
		// TODO mas em vez do yahoo o host name
		he = gethostbyname("www.yahoo.com");
		if (he == NULL) { // do some error checking
			herror("gethostbyname"); // herror(), NOT perror()
			exit(1);
		}

		// print information about this host:
		printf("Official name is: %s\n", he->h_name);

		// TODO mas em vez de imprimir temos que guardar num char* para o endereço IP. Vem em notação dotted
		printf("IP address: %s\n", inet_ntoa(*(struct in_addr*)he->h_addr));

		printf("All addresses: ");
		addr_list = (struct in_addr **)he->h_addr_list;
		for(i = 0; addr_list[i] != NULL; i++) {
			printf("%s ", inet_ntoa(*addr_list[i]));
		}
		printf("\n");

		// get the host name of 66.94.230.32:

		inet_aton("66.94.230.32", &addr);
		he = gethostbyaddr(&addr, sizeof(addr), AF_INET);

		printf("Host name: %s\n", he->h_name);
	*/
	


	// Create TCP socket, and connect it to the address and port specified - usar este código dado, mudando o indicado
	int	sockfd;
	struct	sockaddr_in server_addr;

	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;

	/*TODO 32 bit Internet address network byte ordered - recebe o endereço em notação dotted e converte no formato desejado*/
	server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	/*TODO server TCP port must be network byte ordered - com a port do FTP*/
	server_addr.sin_port = htons(SERVER_PORT);

	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		perror("socket()");
		exit(-1);
	}
	/*connect to the server*/
	if(connect(sockfd,(struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		perror("connect()");
		exit(-1);
	}

	// Função para ler resposta do servidor segundo o especificado no rfc959-FTP.txt secção 4.2.FTP Replies 
	// (também no ficheiro respostaDoServidor_diretrizes)
    
	/*send a string to the server*/
	bytes = write(sockfd, buf, strlen(buf));
	printf("Bytes escritos %d\n", bytes);

	close(sockfd);
	exit(0);
}


