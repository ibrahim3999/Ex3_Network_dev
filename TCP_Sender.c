#include <stdio.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>

#define FILE_SIZE 2*1048576 
#define XOR "0000 0000 0000 0000"
#define M_Authentication_SIZE 21

int send_file(int client_socket, FILE *file, char * algo);
int createSocket();
void generateRandomFile(const char *filename ,int size);

int main(int argc , char *argv[]) {
    int port=-1;
    char *algo=NULL;
    char *IP=NULL;
    if (argc != 7) {
        printf("Usage: ./sender -ip <IP_ADDRESS> -p <PORT> -algo <ALGORITHM>\n");
        return 1;
    }

    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-ip") == 0) {
            IP = argv[i + 1];
        } else if (strcmp(argv[i], "-p") == 0) {
            port = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-algo") == 0) {
            algo = argv[i + 1];
        } else {
            printf("Unknown option: %s\n", argv[i]);
            return 1;
        }
    }

    if (IP == NULL || port == -1 || algo == NULL) {
        printf("All options (-ip, -p, -algo) are required.\n");
        exit(EXIT_FAILURE);
    }

    printf("Port: %d\n", port);
    printf("Algorithm: %s\n", algo);
    printf("IP: %s\n", IP);


    int User_Decision = 1;
    char CheckAuth[M_Authentication_SIZE];
    strcpy(CheckAuth,"0101 0001 1100 0000");
    FILE *file = NULL;
    int count =0,sizeIn_byets=0,oneMbCounter=0;
    char User_Decision_MSG[1]; //  Save the user decision about to send again
    const char *filename = "random_file.txt";
    int file_size = FILE_SIZE; // Specify the size of the file in bytes
    generateRandomFile(filename, file_size);
    
    int client_socket = createSocket(port,IP); // create the client socket

    char buffer[1024];
    int j=0;
    while(1){ // loop to countine until the user want to stop

        file = fopen(filename,"r");

        if(file == NULL){
            perror("Error opening file \n");
            exit(EXIT_FAILURE);
        }

        count++;
        printf("RUN %d\n",count);
        sizeIn_byets = send_file(client_socket,file,algo);

        oneMbCounter+=sizeIn_byets;
        printf("%d\n",oneMbCounter);

        recv(client_socket, &CheckAuth, sizeof(CheckAuth), 0);
        // Check the authentication
        if (strcmp(XOR, CheckAuth) == 0) {
            printf("Authentication success\n");
        }

        if(oneMbCounter == FILE_SIZE){
            printf("sent all the %d MB file (%d byets) \n", FILE_SIZE/1048576,oneMbCounter);
            fclose(file);

            printf("User Decision:-\nPress 1 to continue ,0 to exit!\n");
            scanf("%d", &User_Decision);
            printf("*******************************\n");
            
            if(User_Decision == 0){
                User_Decision_MSG[0]='0';
            
            }else{
                User_Decision_MSG[0]='1';
            }

            printf("\nSender send the Decision of the user\n");
            //send the decision.
            send(client_socket, User_Decision_MSG, sizeof(User_Decision_MSG), 0);
        }else{
            printf("sent just %d out of %d\n", oneMbCounter, FILE_SIZE);
        }

        if (User_Decision==0){
            break;
        }
        j++;
        sizeIn_byets = 0;
        oneMbCounter = 0;
    }

    close(client_socket);
    
    return 0;
}

//Function to generate a file with random content
void generateRandomFile(const char *filename, int size) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Seed the random number generator
    srand(time(NULL));

    // Generate random content and write it to the file
    for (int i = 0; i < size; ++i) {
        // Generate a random character (ASCII value between 0 and 255)
        char random_char = rand() % 256;
        // Write the character to the file
        fputc(random_char, file);
    }

    fclose(file);
}


int createSocket(int port, char *ip) {
    // Create the socket 
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket == -1) {
        printf("Couldn't create the socket\n");
        exit(EXIT_FAILURE);
    }

    // Prepare the address that the network understands
    struct sockaddr_in server_address;
    memset(&server_address,0,sizeof(server_address));//update
    server_address.sin_family = AF_INET;
    server_address.sin_port=htons(port);

    int rval = inet_pton(AF_INET,(const char*)ip,&server_address.sin_addr);
    if(rval <= 0){
        printf("function inet_pton() failed \n");
        return -1;
    }

    // Connect to the server (Receiver).
    int conn = connect(client_socket,(struct  sockaddr *) &server_address,sizeof(server_address));

    if(conn == -1){

        printf("connect() failed\n");
        exit(EXIT_FAILURE);
    }else{
        printf("cilent_connected to server \n");
    }
    
    return client_socket;
}
int send_file(int client_socket, FILE *file,char *algo) {

    char Name_CC_Algo[250];
    char buffer[FILE_SIZE];
    socklen_t length;
    int sizeinbytes=0;
    // By default cc algorithm is cubic
    strcpy(Name_CC_Algo,algo);
    
    if (strcmp(Name_CC_Algo, "cubic") == 0){
        int sock_opt = getsockopt(client_socket, IPPROTO_TCP, TCP_CONGESTION, Name_CC_Algo, &length);
        if( sock_opt != 0) {
            perror("getsockopt");
            exit(EXIT_FAILURE);
        }
        printf("Current CC, type: %s \n", Name_CC_Algo);
    }
       if (strcmp(Name_CC_Algo, "reno") == 0){
        length = sizeof(Name_CC_Algo);
        int sock_opt = setsockopt(client_socket, IPPROTO_TCP, TCP_CONGESTION, Name_CC_Algo, length);
        if (sock_opt != 0) {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        printf("Current CC, type: %s \n", Name_CC_Algo);
    }
    
    fread(buffer,1, sizeof(buffer),file);
    sizeinbytes = send(client_socket, buffer, sizeof(buffer), 0);
    
    if(sizeinbytes<0){
        printf("Send part1 failed with error!\n");
        exit(1);
    }

    return sizeinbytes;
}
