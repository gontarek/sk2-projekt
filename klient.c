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
    char buf[BUF_SIZE];
    struct sockaddr_in myaddr, serv_addr;
    int sock, nBind;

    // struktura adresu własnego
    memset(&myaddr, 0, sizeof(struct sockaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = INADDR_ANY; // jakikolwiek adres
    myaddr.sin_port = 0; // -> jakikolwiek port wybrany przez system

    // struktura adresu serwera
    memset(&serv_addr, 0, sizeof(struct sockaddr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(SERVER_PORT);

    //utworzenie socketa
    sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
   {
       fprintf(stderr, "%s: Can't create a socket.\n", argv[0]);
       exit(1);
   }

    socklen_t serv_addr_size = sizeof(struct sockaddr);

    //zbindowanie socketa
    nBind = bind(sock, (struct sockaddr*)&myaddr, sizeof(struct sockaddr));
    if (nBind < 0)
   {
       fprintf(stderr, "%s: Can't bind a name to a socket.\n", argv[0]);
       exit(1);
   }
   // komunikacja z serwerem
    strcpy(buf, "Hello server!\n");
    sendto(sock, buf, strlen(buf)+1, 0, (struct sockaddr*)&serv_addr, serv_addr_size);
    recvfrom(sock, buf, BUF_SIZE, 0, (struct sockaddr*)&serv_addr, &serv_addr_size);
    printf("%s :: %s", argv[0], buf);
    close(sock);
    return(0);
}
