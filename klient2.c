#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024

int main(int argc, char* argv[])
{
   int nSocket;
   int nConnect;
   int nBytes;
   int nBind;
   int nTmp;
   struct sockaddr_in stServerAddr, stMyAddr;
   struct hostent* lpstServerEnt;
   char cbBuf[BUF_SIZE];

   if (argc != 3)
   {
     fprintf(stderr, "Usage: %s server_name port_number\n", argv[0]);
     exit(1);
   }

   /* look up server's IP address */
   lpstServerEnt = gethostbyname(argv[1]);
   if (! lpstServerEnt)
   {
      fprintf(stderr, "%s: Can't get the server's IP address.\n", argv[0]);
      exit(1);
   }

   /* create a socket */
   nSocket = socket(PF_INET, SOCK_DGRAM, 0);
   if (nSocket < 0)
   {
      fprintf(stderr, "%s: Can't create a socket.\n", argv[0]);
      exit(1);
   }

   /* client info */
   memset(&stMyAddr, 0, sizeof(struct sockaddr));
   stMyAddr.sin_family = AF_INET;
   stMyAddr.sin_addr.s_addr = INADDR_ANY;
   stMyAddr.sin_port = 0;

   /* server info */
   memset(&stServerAddr, 0, sizeof(struct sockaddr));
   stServerAddr.sin_family = AF_INET;
   memcpy(&stServerAddr.sin_addr.s_addr, lpstServerEnt->h_addr, lpstServerEnt->h_length);
   stServerAddr.sin_port = htons(atoi(argv[2]));

   /* bind client name to a socket */
   nBind = bind(nSocket, (struct sockaddr*)&stMyAddr, sizeof(struct sockaddr));
   if (nBind < 0)
   {
       fprintf(stderr, "%s: Can't bind a name to a socket.\n", argv[0]);
       exit(1);
   }

   /* send a data to a server */
   strcpy(cbBuf, "Hello server!\n");
   sendto(nSocket, cbBuf, strlen(cbBuf)+1, 0, (struct sockaddr*)&stServerAddr,
          sizeof(struct sockaddr));

   /* receive a data from a server */
   nTmp = sizeof(struct sockaddr);
   nBytes = recvfrom(nSocket, cbBuf, BUF_SIZE, 0, (struct sockaddr*)&stServerAddr, &nTmp);
   printf("%s:: %s", argv[0], cbBuf);

   close(nSocket);
   return 0;
}
