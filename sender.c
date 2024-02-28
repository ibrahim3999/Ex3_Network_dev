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
#define SERVER_PORT 5346
#define SERVER_IP_ADDRESS "127.0.0.1"
#define SIZEFILE 1048576
#define HALF_SIZEFILE 524288
#define M_Authentication_SIZE 21
#define FIlE_NAME "dataset.txt"
#define XOR "0101 0001 1100 0000"

int CreateSocket();

int send_first_Part(int client_socket, FILE *file);

int send_part2(int client_socket, FILE *file);

int main() {


    char CheckAuth[M_Authentication_SIZE];
    strcpy(CheckAuth,"0101 0001 1100 0000");
    FILE *file = NULL;


    int count=0;
    int User_Decision = 1;
    int SizeIn_bytes = 0;
    int OneMb_Counter=0;
    char User_Decision_MESSAGE[1]; // to save in the message we get from the user.


    int client_socket = CreateSocket();
    int j = 0;
    while(1) { // loop to continue until the user want to exit.
        // 1 to continue , 0 to stop.
    //create and open the file we well-read the data from.
            file = fopen(FIlE_NAME, "r");
            if(file == NULL) {
                printf("Failed to open file dataset.txt\n");
                exit(EXIT_FAILURE);
            }

                count++;
                printf("RUN %d\n",count);



                SizeIn_bytes = send_first_Part(client_socket,file);
                // the print only to help us that send true.
                OneMb_Counter+=SizeIn_bytes;
                printf("%d\n",OneMb_Counter);



                recv(client_socket, &CheckAuth, sizeof(CheckAuth), 0);
                // Check the authentication
                if (strcmp(XOR, CheckAuth) == 0) {
                    printf("Authentication success\n");
                }


                    printf("Change the cc algorithm to reno\n");
                    SizeIn_bytes = send_part2(client_socket,file);
                    // the print only to help us that send true.
                    OneMb_Counter += SizeIn_bytes;
                    // the print only to help us that send true.

                    printf("%d\n", OneMb_Counter);

                    if (OneMb_Counter == SIZEFILE) {



                        printf("sent all the 1MB file: %d\n", OneMb_Counter);
                        fclose(file);
                        printf("User Decision:-\n click 1 to continue ,0(Zero) to exit!\n");

                        scanf("%d", &User_Decision);


                        printf("*******************************\n");


                        if (User_Decision == 0) { // here check what the user put decision

                            User_Decision_MESSAGE[0] = '0';

                        } else {
                            User_Decision_MESSAGE[0] = '1';

                        }


                        printf("\nSender send the Decision of the user\n");
                        //send the decision.
                        send(client_socket, User_Decision_MESSAGE, sizeof(User_Decision_MESSAGE), 0);




                    } else {
                        printf("sent just %d out of %d\n", OneMb_Counter, SIZEFILE);

                    }

                    if (User_Decision==0){
                        break;
                    }

                        j++;
                        SizeIn_bytes = 0;
                        OneMb_Counter = 0;
                }

            //In the end close the connection.
            close(client_socket);

            return 0;
        }





int send_part2(int client_socket, FILE *file) {

    int j=0;
    int sizeinbytes=0;
    char Name_CC_Algo[250];
    char buffer[HALF_SIZEFILE];
    socklen_t length;

    strcpy(Name_CC_Algo, "reno");
    length = sizeof(Name_CC_Algo);
    int set_sock_opt = setsockopt(client_socket, IPPROTO_TCP, TCP_CONGESTION, Name_CC_Algo, length);
    if (set_sock_opt != 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }


    printf("\nclient (%d) Current CC, type: %s  \n\n", j, Name_CC_Algo);



    bzero(buffer, sizeof(buffer)); // update...
    printf("Send the second part of the file\n\n");

    //read and send the next half size of the file.
    fread(buffer, 1, sizeof(buffer), file);
    sizeinbytes = send(client_socket, buffer, sizeof(buffer), 0);

    if (sizeinbytes < 0) {
        printf("Send part2 failed with error!\n");
        exit(1);
    }


    return sizeinbytes;
}


int send_first_Part(int client_socket, FILE *file) {

    char Name_CC_Algo[250];
    char buffer[HALF_SIZEFILE];
    socklen_t length;
    int sizeinbytes=0;
    // By default cc algorithm is cubic
            strcpy(Name_CC_Algo,"cubic");
            int get_sock_opt = getsockopt(client_socket, IPPROTO_TCP, TCP_CONGESTION, Name_CC_Algo, &length);
            if( get_sock_opt != 0) {
                perror("getsockopt");
                exit(EXIT_FAILURE);
            }




                printf("Send the first part of the file\n");
                //read half-size of the data and dave in buffer than send.
                fread(buffer,1, sizeof(buffer),file);
                sizeinbytes = send(client_socket, buffer, sizeof(buffer), 0);
                if(sizeinbytes<0){
                    printf("Send part1 failed with error!\n");
                    exit(1);
                }

    return sizeinbytes;
}


int CreateSocket() {


    //create the socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket == -1) {
        printf("Couldn't create the socket : \n");
        exit(EXIT_FAILURE); // failing exit status.
    }

    //prepare the address,that well network understand.
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));//update
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    int rval = inet_pton(AF_INET, (const char*)SERVER_IP_ADDRESS, &server_address.sin_addr);
    if(rval <= 0) {
        printf("inet_pton() failed");
        return -1;
    }

    // Connect to the server (Receiver).
    int connection = connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if(connection == -1) {
        printf("connect() failed with error code:\n");
        exit(EXIT_FAILURE); // failing exit status.
    }
    else {
        printf("client connected to server!\n");
    }

    return client_socket;

}


