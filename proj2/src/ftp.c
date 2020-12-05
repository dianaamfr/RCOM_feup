#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ftp.h"

void printArgs(ftp_args * args){
    printf("****************************\n");
    printf("User: %s\n", args->user);
    printf("Password: %s\n", args->pass);
    printf("Host: %s\n", args->host);
    printf("IP Address: %s\n", args->ipAddr);
    printf("File path: %s\n", args->file_path);
    printf("File name: %s\n\n", args->file_name);
    printf("****************************\n");
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
        memset(args->user, 0, MAX_SIZE);
        strcpy(args->user, "anonymous");
        memset(args->pass, 0, MAX_SIZE);
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
    memset(args->host, 0, MAX_SIZE);
    strcpy(args->host, token);

    // file
    token = strtok(NULL, "\0");
    if (token == NULL) {
        return -1;
    }

    // find the last / of the path
    int last_bar = -1;
    for(int i = strlen(token) - 1; i >= 0 ; i--) {
        if(token[i] == '/'){
            last_bar = i;
            break;
        }
    }

    memset(args->file_path, 0, MAX_SIZE);
    memset(args->file_name, 0, MAX_SIZE);

    if (last_bar != -1) {
        int file_name_len = strlen(token) - last_bar - 1;

        strncpy(args->file_path, token, last_bar);
        strncpy(args->file_name, &token[last_bar + 1], file_name_len);

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

    printf("Connected to socket with success\n\n");

    return sockfd; // Returns control socket fd on success
}


void readReplyFTP(int control_sock_fd, char * reply) {
    
    FILE * fp = fdopen(control_sock_fd, "r");
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


int loginFTP(int control_sock_fd, char * username, char * password){
    
    char reply[MAX_SIZE];
    char reply_first_digit;

    // Send username
    reply_first_digit = commandAndReplyFTP(control_sock_fd, "USER", username, reply);
    if((reply_first_digit != POSITIVE_INTERMEDIATE) && (reply_first_digit != POSITIVE_COMPLETION)){
        fprintf(stderr,"Error sending username\n");
        return -1;
    }

    // Send password
    if((reply_first_digit = commandAndReplyFTP(control_sock_fd, "PASS", password, reply)) != POSITIVE_COMPLETION){
        fprintf(stderr,"Error sending password\n");
        return -1;
    }   
    
    return 0;
}


int sendCommandFTP(int control_sock_fd, char * command, char * cmd_args) {
    printf("\nSending Command: %s %s\n", command, cmd_args);

    char full_command[MAX_SIZE];
    int res = snprintf(full_command, MAX_SIZE, "%s %s\n", command, cmd_args);
    if(res < 0 || res > MAX_SIZE){
        perror("snprintf() building command error");
        return -1;
    }

    int nw;
    if((nw = write(control_sock_fd, full_command, strlen(full_command))) != strlen(full_command)){
        perror("write() writing command error");
        return -1;
    }

    return 0;
}


char commandAndReplyFTP(int control_sock_fd,  char * command, char * cmd_args, char * reply){
    
    if (sendCommandFTP(control_sock_fd, command, cmd_args) == -1) {
        return -1;
    }
    
    char  reply_first_digit;

    while (1) {

        readReplyFTP(control_sock_fd, reply);
        reply_first_digit = reply[0];
        
        switch (reply_first_digit) {
            case TRANSIENT_NEGATIVE_COMPLETION:     // Try again  
                if (sendCommandFTP(control_sock_fd, command, cmd_args) == -1) {
                    return -1;
                }
                break;
            case PERMANENT_NEGATIVE_COMPLETION:     // Error
                close(control_sock_fd);
                return reply_first_digit;
            default: 
                return reply_first_digit;
        }
    }

}


int cwdFTP(int control_sock_fd, char * path){

    if(strlen(path) == 0) return 0;

    char reply[MAX_SIZE];
    char reply_first_digit;
    

    if((reply_first_digit = commandAndReplyFTP(control_sock_fd, "CWD", path, reply)) != POSITIVE_COMPLETION){
        fprintf(stderr,"Error changing working directory\n");
        return -1;
    }

    return 0;
}


int pasvFTP(int control_sock_fd) {
    
    char reply[MAX_SIZE];
    char reply_first_digit;

	if ((reply_first_digit = commandAndReplyFTP(control_sock_fd, "PASV", "",reply)) != POSITIVE_COMPLETION) {
		fprintf(stderr,"Error entering passive mode\n");
		return -1;
	}

    char * token;
    int pasvReply[6];
    memset(pasvReply,0, 6 * sizeof(int));
    
    if ((token = strtok(reply,"(")) == NULL) {
        return -1;
    }

    // parse reply
    int i = 0;
    while(i < 6) {

        if(i == 5) {
            token = strtok(NULL, ").");
        }
        else{
            token = strtok(NULL, ",");
        }

        pasvReply[i] = atoi(token);
        i++;
    }

    // parse server ip
    char serverIP[MAX_SIZE];
	if ((sprintf(serverIP, "%d.%d.%d.%d", pasvReply[0], pasvReply[1], pasvReply[2], pasvReply[3])) < 0) {
		fprintf(stderr, "Error building IP address in passive mode\n");
		return -1;
	}

	// parse port
	int port = pasvReply[4] * 256 + pasvReply[5];

    // create new socket
    int data_sock_fd;
    if ((data_sock_fd = connectFTP(port, serverIP)) == -1) {
        return -1;
    }

	return data_sock_fd;
}


int retrFTP(int control_sock_fd, char * file_name) {
    
    char reply[MAX_SIZE];
    if(commandAndReplyFTP(control_sock_fd, "RETR", file_name, reply) < 0){
        printf("Error sending Command Retr\n");
        return -1;
    }
	return 0;
}  


int saveFile(int data_sock_fd, char * file_name){
    int fd;

    if ((fd = open(file_name, O_WRONLY | O_CREAT, 0666)) < 0) {
		fprintf(stderr, "Error creating the file\n");
		return -1;
	}

    char buffer[MAX_SIZE];
    int nr, nw;
    while ((nr = read(data_sock_fd, buffer, MAX_SIZE))) {
		if ((nw = write(fd, buffer, nr)) < 0) {
			fprintf(stderr,"Error writing file\n");
			return -1;
		}
	}

    if(close(fd) < 0){
        fprintf(stderr,"Error closing file\n");
        return -1;
    }

    return 0;
}


int quitFTP(int control_sock_fd) {
	
	char reply[MAX_SIZE];
    if(commandAndReplyFTP(control_sock_fd, "QUIT", "", reply) != POSITIVE_COMPLETION){
        printf("Error sending Command QUIT\n");
        return -1;
    }

	return 0;
}

