#define MAX_SIZE 1024
#define FTP_PORT 21

typedef struct ftp_args {
    char user[MAX_SIZE];
    char pass[MAX_SIZE];
    char host[MAX_SIZE];
    char file_path[MAX_SIZE];
    char file_name[MAX_SIZE];
    char ipAddr[MAX_SIZE];
} ftp_args;

typedef enum FTP_REPLY {
    POSITIVE_PRELIMINARY = '1',
    POSITIVE_COMPLETION = '2',
    POSITIVE_INTERMEDIATE = '3',
    TRANSIENT_NEGATIVE_COMPLETION = '4',
    PERMANENT_NEGATIVE_COMPLETION = '5'
} FTP_REPLY;


void printArgs(ftp_args * args);

// TODO -  rever pq há leituras que dão segmentation fault em vez de dar erro qnd os args nao seguem o formato
int readArgs(ftp_args * args, char * argv);


int getIPFromHostName(char * host_name, char * ipAddr);


int connectFTP(int port, const char * ipAddr);


void readReplyFTP(int sock_fd, char * reply);


int loginFTP(int sock_fd, char * username, char * password);


int sendCommandFTP(int sock_fd, char * command, char * cmd_args);


char commandAndReplyFTP(int sock_fd,  char * command, char * cmd_args, char * reply);


int cwdFTP(int sock_fd, char * path);


/*
Na resposta, o servidor envia 6 bytes (exemplo: 193,136,28,12,19,91) com o seguinte significado:
- 193,136,28,12 endereço IP do servidor,
- 19,91 porta em que o servidor se encontra à espera de ligação.
A interpretação destes dois bytes deverá ser feita daseguinte forma:
porta = 19 * 256 + 91 = 4955
*/
// RETORNA DATA SOCKET
int pasvFTP(int control_sock_fd);

/*
This command causes the server-DTP to transfer a copy of the
file, specified in the pathname, to the server- or user-DTP
at the other end of the data connection.  The status and
contents of the file at the server site shall be unaffected.
*/
int retrFTP(int control_sock_fd, char * file_name);


int saveFile(int data_sock_fd, char * file_name);


int quitFTP(int control_sock_fd);
