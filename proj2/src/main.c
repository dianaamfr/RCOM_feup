#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int	sockfd;

#define MAX_SIZE 256
#define FTP_PORT 21

typedef struct ftp_args {
    char user[MAX_SIZE];
    char pass[MAX_SIZE];
    char host[MAX_SIZE];
    char file_path[MAX_SIZE];
    char file_name[MAX_SIZE];
    char ipAddr[MAX_SIZE];
} ftp_args;


typedef struct ftp {
    int control_socket; 
    int data_socket;
} ftp;


typedef enum FTP_REPLY {
    POSITIVE_PRELIMINARY = '1',
    POSITIVE_COMPLETION = '2',
    POSITIVE_INTERMEDIATE = '3',
    TRANSIENT_NEGATIVE_COMPLETION = '4',
    PERMANENT_NEGATIVE_COMPLETION = '5'
} FTP_REPLY;

void printArgs(ftp_args * args){
    printf("User: %s\n", args->user);
    printf("Password: %s\n", args->pass);
    printf("Host: %s\n", args->host);
    printf("IP Address: %s\n", args->ipAddr);
    printf("File path: %s\n", args->file_path);
    printf("File name: %s\n\n", args->file_name);
    
}

int readArgs(ftp_args * args, char * argv) {
    
    // ftp
    char * token = strtok(argv,":");
    if ((token == NULL) || (strcmp(token, "ftp") != 0)) {
        return -1;
    }

    // rest of argv
    token = strtok(NULL, "\0");
    char rest[MAX_SIZE];
    strcpy(rest, token);

    // user
    char rest_copy[MAX_SIZE];
    strcpy(rest_copy, rest);
    token = strtok(rest_copy, ":");

    if (token == NULL || (token[0] != '/') || (token[1] != '/') || (strlen(token) < 2)) {
        return -1;
    }

    if (strcmp(token, rest) == 0) {
        // user & pass not specified => anonymous
        strcpy(args->user, "anonymous");
        strcpy(args->pass, "");
    
        memset(rest_copy, 0, MAX_SIZE);
        strcpy(rest_copy, &rest[2]);
        memset(rest, 0, MAX_SIZE);
        strcpy(rest, rest_copy);
 
    }
    else {
        // user
        strcpy(args->user, &token[2]);
        // pass
        token = strtok(NULL, "@");
        if (token == NULL || (strlen(token) == 0)) {
            return -1;
        }
        strcpy(args->pass, token);

        token = strtok(NULL, "\0");
        memset(rest, 0, MAX_SIZE);
        strcpy(rest, token);
    }

    // host
    token = strtok(rest, "/");    
    if (token == NULL) {
        return -1;
    }
    strcpy(args->host, token);

    // file
    token = strtok(NULL, "\0");
    if (token == NULL) {
        return -1;
    }

    char * last_bar = strrchr(token, '/');

    if (last_bar != NULL) {
        char * file_start_ptr = last_bar + 1;

        strncpy(args->file_path, token, last_bar - token);
        strcpy(args->file_name, file_start_ptr);
    }
    // no bars => only filename
    else {
        strcpy(args->file_path, "");
        strcpy(args->file_name, token);
    }

    // set ip to empty string for now
    strcpy(args->ipAddr,"");

    // printArgs(args);
    
    return 0;
}


int getIPFromHostName(char * host_name, char * ipAddr){

    struct hostent *he;
    if((he = gethostbyname(host_name)) == NULL){
        herror("gethostbyname");
        return -1;
    }   

    strcpy(ipAddr, inet_ntoa(*((struct in_addr *)he->h_addr)));

    return 0;
}

int connectFTP(int port, const char * ipAddr){
    int	sockfd;
    struct	sockaddr_in server_addr;

    // Server address handling
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;

	// 32 bit Internet address network byte ordered
	server_addr.sin_addr.s_addr = inet_addr(ipAddr);

	// Server TCP port must be network byte ordered
	server_addr.sin_port = htons(port);

	// Open a TCP socket
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		perror("socket()");
		return -1;
	}
	// Connect to the server
	if(connect(sockfd,(struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		perror("connect()");
		return -1;
	}

    printf("Connection phase completed\n\n");

    return sockfd; // Returns control socket fd on success
}

int sendCommandFTP(ftp *ftp, char * command, char * cmd_args) {
    printf("\nSending Command: %s %s\n", command, cmd_args);

    char full_command[MAX_SIZE];
    int res = snprintf(full_command, MAX_SIZE, "%s %s\n", command, cmd_args);
    if(res < 0 || res > MAX_SIZE){
        perror("snprintf() building command error");
        return -1;
    }

    int nw;
    if((nw = write(ftp->control_socket, full_command, strlen(full_command))) != strlen(full_command)){
        perror("write() writing command error");
        return -1;
    }

    return 0;
}

void readReplyFTP(ftp * ftp, char * reply) {
    
    FILE * fp = fdopen(ftp->control_socket, "r");
    int readReply = 1;

    while(readReply){
        // Clean last reply read
        memset(reply, 0, MAX_SIZE);
        // Read reply
        reply = fgets(reply, MAX_SIZE, fp);
        // Print reply
        printf("%s", reply);

        // Wait for 3-digit code xyz reply followed by a space, with x in [1,5]
        readReply =  (reply[0] < '1' || reply[0] > '5' || reply[3] != ' ');
    }

}

char commandAndReplyFTP(ftp * ftp,  char * command, char * cmd_args){
    
    if (sendCommandFTP(ftp, command, cmd_args) == -1) {
        return -1;
    }
    
    char reply[MAX_SIZE];
    char  reply_first_digit;

    while (1) {

        readReplyFTP(ftp, reply);
        reply_first_digit = reply[0];
        
        switch (reply_first_digit) {
            case POSITIVE_PRELIMINARY:               
                break;
            case TRANSIENT_NEGATIVE_COMPLETION:     // Try again  
                if (sendCommandFTP(ftp, command, cmd_args) == -1) {
                    return -1;
                }
                break;
            case PERMANENT_NEGATIVE_COMPLETION:     // Error
                close(ftp->control_socket);
                return reply_first_digit;
            default: 
                return reply_first_digit;
        }
    }

}

int loginFTP(ftp * ftp, char * username, char * password){
    
    char reply;

    // Send username
    if((reply = commandAndReplyFTP(ftp, "USER", username)) != POSITIVE_INTERMEDIATE){
        fprintf(stderr,"Error sending username");
        return -1;
    }

    // Send password
    if((reply = commandAndReplyFTP(ftp, "PASS", password)) != POSITIVE_COMPLETION){
        fprintf(stderr,"Error sending username");
        return -1;
    }   
    
    return 0;
}


int main(int argc, char** argv){

    ftp_args args;

    // Read & Validate Arguments
    if (argc != 2 || (readArgs(&args, argv[1]) == -1)) {
        fprintf(stderr, "Usage: %s %s\n", argv[0], "ftp://[<user>:<password>@]<host>/<url-path>");
        return -1;
    }


    // Get IP address from Host Name
    if(getIPFromHostName(args.host, args.ipAddr) == -1){
        return -1;
    }

    printArgs(&args);


    // Connect TCP socket to the address and FTP port
    ftp ftp;
    if((ftp.control_socket = connectFTP(FTP_PORT, args.ipAddr)) == -1){
        fprintf(stderr, "connectFTP() error");
        return -1;
    }


    // Read Server reply
    char serverReply[MAX_SIZE];
    readReplyFTP(&ftp, serverReply);
    if(serverReply[0] != POSITIVE_COMPLETION){
        fprintf(stderr, "Connection failed");
    }


    // Login
    loginFTP(&ftp, args.user, args.pass);

    return 0;

}
