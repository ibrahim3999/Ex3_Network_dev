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


//#define SERVER_PORT 12345
#define SERVER_IP "127.0.0.1"
#define FILE_SIZE 2*1048576 
#define XOR "0101 0001 1100 0000"
#define M_Authentication_SIZE 21


int setSock(int port){


    struct sockaddr_in server_addr;
    char namealgo[250];
    socklen_t length;
    // The  memset()  function  fills  the  first  n  bytes of the memory area
    //       pointed to by s with the constant byte c.
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port); // short, network byte order

    /*socket()  creates  an endpoint for communication and returns a file de‐
    scriptor that refers to that endpoint.  The file descriptor returned by
    a  successful call will be the lowest-numbered file descriptor not cur‐
    rently open for the process.*/
    // create a socket lisener.
    int recevier_socket = -1;
    if((recevier_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Couldn't create a socket listener : %d\n",errno);
        perror("socket\n");
        return -1;
    }

    /* getsockopt()  and  setsockopt()  manipulate  options for the socket re‐
       ferred to by the file descriptor sockfd.  Options may exist at multiple
       protocol levels; they are always present at the uppermost socket level.*/
    strcpy(namealgo,"cubic");
    length = sizeof(namealgo);
    if (getsockopt(recevier_socket,IPPROTO_TCP,TCP_CONGESTION,namealgo,&length)!=0){

        perror("getsockopt\n");
        return -1;
    }

    /*When a socket is created with socket(2), it exists in a name space (ad‐
       dress family) but has no address assigned to it.   bind()  assigns  the
       address  specified  by  addr  to the socket referred to by the file de‐
       scriptor sockfd.  addrlen specifies the size, in bytes, of the  address
       structure  pointed to by addr.  Traditionally, this operation is called
       “assigning a name to a socket”.*/
    // connect the server to a port which can read and write on.
    if(bind(recevier_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        printf("Bind failed with error code : %d" , errno);
        return -1;
    }

    printf("the server is ready!\n\n");

    /*listen()  marks  the  socket referred to by sockfd as a passive socket,
       that is, as a socket that will be used to  accept  incoming  connection
       requests using accept(2).

       The  sockfd  argument  is  a file descriptor that refers to a socket of
       type SOCK_STREAM or SOCK_SEQPACKET.*/
    if(listen(recevier_socket, 500) == -1) {
        printf("listen() failed with error code : %d",errno);
        return -1;
    }

    return recevier_socket; // return the socket that we create.
}

void printTimes(double *time, int len,double sum) {

    printf("----------------------------------\n");
    printf("- * Statistics * -\n");
    for (int j = 0; j < len; j++) {
        printf("- RUN #%d Data : Time %.6f\n",j+1,*(time + j));
    }
    double avg = sum / len;
    double bandwidth = len / (avg * 1024 * 1024);

    printf("-\n");
    printf("- Average time:\t%0.6f\n", avg);
    printf("- Average bandwidth : %.6f \n",bandwidth);

    printf("----------------------------------\n");
    free(time);

}


int main(int argc , char *argv[]){

   // on linux to prevent crash on closing socket.
    signal(SIGPIPE, SIG_IGN);

    int port=-1;
    char *algo=NULL;

     if (argc != 5) {
        fprintf(stderr, "Usage: %s -p PORT -algo ALGORITHM \n", argv[0]);
        exit(EXIT_FAILURE);
    }
    for (int i = 1; i < argc; i += 1) {
    
        if (strcmp(argv[i], "-p") == 0) {
            port = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-algo") == 0) {
            algo = argv[i + 1];
        } 
    }

    if (port == -1 || algo[0] == '\0') {
        fprintf(stderr, "Both port and algorithm must be provided.\n");
        fprintf(stderr, "Usage: %s -p PORT -algo ALGO\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    printf("Port: %d ", port);
    printf(" Algorithm: %s\n", algo);
    printf("Starting Receiver...\n");
    printf("Waiting for TCP connection...\n");


    int size2=0;
    // on linux to prevent crash on closing socket.
    signal(SIGPIPE, SIG_IGN);
    char buffer[FILE_SIZE];
    char User_Decision_MESSAGE[1];
    char Auth_Message[M_Authentication_SIZE]="0101 0001 1100 0000";
    char Check_DECISION[1];// create the message exit that the receiver will send if the user want to exit.
    Check_DECISION[0]='0';

    //Accept and incoming connection
    printf("Waiting for incoming connections\n");

    struct sockaddr_in client_addr;
    socklen_t client_addr_length = sizeof(client_addr);

    double sum_for_average= 0.0;

      // allocating memory
    double *arrTime = NULL;
    arrTime= (double *) malloc(1);

    char Name_CC_Algo[250]; // to the name of the cc algorithm
    socklen_t length;


    User_Decision_MESSAGE[0];
    int recevier_socket = -1;
    int countbytes = 0;

    int c = 0;
    int Bole=1;
    int rcv = 0;
    int t=1;

    recevier_socket = setSock(port); // create the socket
    if (recevier_socket == -1) {
        exit(EXIT_FAILURE);
    }

    do{
        printf("Sender connected, beginning to receive file...");
        countbytes=0;
        memset(&client_addr, 0, sizeof(client_addr)); 
        client_addr_length = sizeof(client_addr);

         // accept a connection from the sender
        int client_socket = accept(recevier_socket, (struct sockaddr *) &client_addr, &client_addr_length);
        if (client_socket == -1) {
            printf("listen failed with error code : \n");

            return -1;
        } else {
            printf("\nclient number connection accepted\n");
        }

        while (1)
        {
            struct timeval start, end;
            gettimeofday(&start, 0); 
            rcv = recv(client_socket, &buffer, sizeof(buffer), 0);

            countbytes += rcv;
            printf("Received : %d bytes \n",countbytes);

            if(countbytes == FILE_SIZE){
                gettimeofday(&end, 0); // end measure

                printf("File transfer completed.\n");
                printf("Waiting for Sender response...\n");
                printf("Sending the Authentication\n");
                send(client_socket,Auth_Message, sizeof(M_Authentication_SIZE),0);
                double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1e6;

                sum_for_average += time_spent;
                *(arrTime + c) = time_spent;
                c++;

                printf("Time in microseconds: %f microseconds\n", time_spent);
                
                size2+=countbytes;
                countbytes=0;


                arrTime = (double *) realloc(arrTime, sizeof(double) * (c + 1));

                // To get the decision of the user
                bzero(User_Decision_MESSAGE, sizeof(User_Decision_MESSAGE));// update
                printf("\n\n");
                // get the message option from the user(sender)
                recv(client_socket, &User_Decision_MESSAGE, sizeof(User_Decision_MESSAGE), 0);
               
                if (User_Decision_MESSAGE[0] == Check_DECISION[0]) { 
                // if it's equal to exit-message then stop
                    Bole =0;
                    break;
                }
                 else{ //else continue
                        Bole =1;
                        size2=0;
                        printf("Receive %d\n",++t);
                    }

            }


        }
        
    }while(Bole);

    printTimes(arrTime,c,sum_for_average);
    close(recevier_socket);
    return 0;
}

