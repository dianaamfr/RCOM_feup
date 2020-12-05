#include <stdio.h>
#include <unistd.h>
#include "ftp.h"
 

int main(int argc, char** argv){

    ftp_args args;

    // Read & Validate Arguments
    if (argc != 2 || (readArgs(&args, argv[1]) == -1)) {
        fprintf(stderr, "Usage: %s %s\n", argv[0], "ftp://[<user>:<password>@]<host>/<url-path>\n");
        return -1;
    }


    // Get IP address from Host Name
    if(getIPFromHostName(args.host, args.ipAddr) == -1){
        return -1;
    }

    printArgs(&args);


    // Connect TCP socket to the address and FTP port
    int control_sock_fd; 
    if((control_sock_fd = connectFTP(FTP_PORT, args.ipAddr)) == -1){
        fprintf(stderr, "connectFTP() error\n");
        return -1;
    }

    
    // Read Server reply
    char serverReply[MAX_SIZE];
    readReplyFTP(control_sock_fd, serverReply);
    if(serverReply[0] != POSITIVE_COMPLETION){
        fprintf(stderr, "Connection failed\n");
    }

    // Login - USER & PASS
    if(loginFTP(control_sock_fd, args.user, args.pass) == -1){
        return -1;
    }

    // Change Working Directory - CWD
    if(cwdFTP(control_sock_fd, args.file_path) == -1){
        return -1;
    }

    int data_sock_fd; 
    // Passive Mode - PASV
	if ((data_sock_fd = pasvFTP(control_sock_fd)) == -1) {
		return -1;
	}

    // RETRIEVE (RETR)
    if(retrFTP(control_sock_fd, args.file_name) == -1){
        return -1;
    }

    // Save file
    if(saveFile(data_sock_fd, args.file_name) == -1){
        return -1;
    }

    if(close(data_sock_fd) < 0){
        fprintf(stderr,"Error closing data socket\n");
        return -1;
    }

    if(quitFTP(control_sock_fd)){
        return -1;
    }

    if(close(control_sock_fd) < 0){
        fprintf(stderr,"Error closing data socket\n");
        return -1;
    }

    return 0;

}
