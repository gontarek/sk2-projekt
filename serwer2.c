#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encrypt.h"

#define QUEUE_SIZE 5
#define BUF_SIZE 1024

int main(int argc, char* argv[])
{
   int nSocket;
   int nBind;
   int key = 10;
   int nFoo = 1, nTmp;
   struct sockaddr_in stMyAddr, stClientAddr;
   char cbBuf[BUF_SIZE];

   /* address structure */
   memset(&stMyAddr, 0, sizeof(struct sockaddr));
   stMyAddr.sin_family = AF_INET;
   stMyAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   stMyAddr.sin_port = htons(atoi(argv[1]));

   /* create a socket */
   nSocket = socket(AF_INET, SOCK_DGRAM, 0);
   if (nSocket < 0)
   {
       fprintf(stderr, "%s: Can't create a socket.\n", argv[0]);
       exit(1);
   }
   setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&nFoo, sizeof(nFoo));

   /* bind a name to a socket */
   nBind = bind(nSocket, (struct sockaddr*)&stMyAddr, sizeof(struct sockaddr));
   if (nBind < 0)
   {
       fprintf(stderr, "%s: Can't bind a name to a socket.\n", argv[0]);
       exit(1);
   }

   while(1)
   {
       /* receive a data from a client */
       nTmp = sizeof(struct sockaddr);
       recvfrom(nSocket,cbBuf, BUF_SIZE, 0, (struct sockaddr*)&stClientAddr, &nTmp);
       printf("%s:: %s", argv[0], cbBuf);

       /* send a data to a client */
       strcpy(cbBuf, "Hello previous server!\n");
       sendto(nSocket, cbBuf, strlen(cbBuf)+1, 0, (struct sockaddr*)&stClientAddr, nTmp);
   }

   close(nSocket);
   return(0);
}
