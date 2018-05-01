#include <stdio.h> //printf(), fprintf(), perror()
#include <sys/socket.h> //socket(), connect(), recv()
#include <arpa/inet.h> // struct sockaddr_in, struct sockaddr, inet_ntoa(), inet_aton()
#include <stdlib.h> //atoi(), exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <string.h> //memset()
#include <unistd.h> //close()

#define MSGSIZE 32
#define MAX_MSGSIZE 1024
#define BUFSIZE (MSGSIZE + 1)

int main(int argc, char* argv[]) {

    int sock; //local socket descripter
    struct sockaddr_in servSockAddr; //server internet socket address
    unsigned short servPort; //server port number
    char recvBuffer[BUFSIZE]; //receive temporary buffer
    int byteRcvd, totalBytesRcvd; //received buffer size

    if (argc != 3) {
        fprintf(stderr, "argument count mismatch error.\n");
        exit(EXIT_FAILURE);
    }

    memset(&servSockAddr, 0, sizeof(servSockAddr));

    servSockAddr.sin_family = AF_INET;

    if (inet_aton(argv[1], &servSockAddr.sin_addr) == 0) {
        fprintf(stderr, "Invalid IP Address.\n");
        exit(EXIT_FAILURE);
    }

    if ((servPort = (unsigned short) atoi(argv[2])) == 0) {
        fprintf(stderr, "invalid port number.\n");
        exit(EXIT_FAILURE);
    }
    servSockAddr.sin_port = htons(servPort);

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ){
        perror("socket() failed.");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr*) &servSockAddr, sizeof(servSockAddr)) < 0) {
        perror("connect() failed.");
        exit(EXIT_FAILURE);
    }

    printf("connect to %s\n", inet_ntoa(servSockAddr.sin_addr));

    totalBytesRcvd = 0;
    while (totalBytesRcvd < MAX_MSGSIZE) {
        if ((byteRcvd = recv(sock, recvBuffer, MSGSIZE, 0)) > 0) {
            recvBuffer[byteRcvd] = '\0';
            printf("%s", recvBuffer);
            totalBytesRcvd += byteRcvd;
        } else if(byteRcvd == 0){
            perror("ERR_EMPTY_RESPONSE");
            exit(EXIT_FAILURE);
        } else {
            perror("recv() failed.");
            exit(EXIT_FAILURE);
        }
    }
    printf("\n");

    close(sock);

    return EXIT_SUCCESS;
}