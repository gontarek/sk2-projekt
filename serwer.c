#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#define BUF_SIZE 1024
#define SERVER_PORT 1234

int main(int argc, char* argv[])
{

    struct sockaddr_in myaddr, client_addr;
    int nFoo = 1;
    //int client_addr_size;
    int nBind;
    int sock;
    char buf[BUF_SIZE];

    // struktura adresu serwera
    memset(&myaddr, 0, sizeof(struct sockaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = INADDR_ANY;
    myaddr.sin_port = htons(SERVER_PORT);

    //tworzenie socketa
    sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
   {
       fprintf(stderr, "%s: Can't create a socket.\n", argv[0]);
       exit(1);
   }
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&nFoo, sizeof(nFoo));

    //zbindowanie socketa
    nBind = bind(sock, (struct sockaddr*)&myaddr, sizeof(struct sockaddr));
    if (nBind < 0)
   {
       fprintf(stderr, "%s: Can't bind a name to a socket.\n", argv[0]);
       exit(1);
   }

    while(1) {
        // otrzymanie danych od klienta
        socklen_t client_addr_size = sizeof(struct sockaddr);
        //client_addr_size = sizeof(struct sockaddr);
        recvfrom(sock, buf, BUF_SIZE, 0, (struct sockaddr*)&client_addr, &client_addr_size);
        printf("%s :: %s", argv[0], buf);
        // wysłanie odpowiedzi
        strcpy(buf, "Hello client!\n");
        sendto(sock, buf, strlen(buf)+1, 0, (struct sockaddr*)&client_addr, client_addr_size);

    }

    close(sock);
    return(0);
}
