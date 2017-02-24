#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "encrypt.h"
#include "route.h"

#define BUF_SIZE 1024



int main(int argc, char* argv[])
{
   int nSocket;
   //int bcast_sock;
   int nConnect;
   int nBytes;
   int nBind;
   //int bcast_bind;
   int nTmp;
   struct sockaddr_in stServerAddr, stMyAddr, stBcAddr;
   struct hostent* lpstServerEnt;
   //struct hostent*  broadcast;
   char cbBuf[BUF_SIZE], keyBuf[BUF_SIZE], intBuf[BUF_SIZE];
   srand(time(NULL));
   int ports[] = {1230, 1231, 1232, 1233, 1234, 1235, 1236, 1237, 1238, 1239};
   int keys[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
   int * route, portsList[3];
   bool connect[] = {false, false, false, false, false, false, false, false, false, false};
   //int r = rand();

   if (argc != 2)
   {
     fprintf(stderr, "Usage: %s server_name\n", argv[0]);
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

   /* create a socket broadcast*/
   /*bcast_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
   if (bcast_sock < 0)
   {
      fprintf(stderr, "%s: Can't create a socket.\n", argv[0]);
      exit(1);
   }
   int broadcastEnable=1;
   int ret=setsockopt(bcast_sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
   if (ret) {
        fprintf(stderr,"Error: Could not open set socket to broadcast mode\n", argv[0]);
        close(bcast_sock);
    }*/

   /* client info */
   memset(&stMyAddr, 0, sizeof(struct sockaddr));
   stMyAddr.sin_family = AF_INET;
   stMyAddr.sin_addr.s_addr = INADDR_ANY;
   stMyAddr.sin_port = 0;

   /* server info */
   memset(&stServerAddr, 0, sizeof(struct sockaddr));
   stServerAddr.sin_family = AF_INET;
   memcpy(&stServerAddr.sin_addr.s_addr, lpstServerEnt->h_addr, lpstServerEnt->h_length);

   /* broadcast server info */
   /*memset(&stBcAddr, 0, sizeof(struct sockaddr));
   stBcAddr.sin_family = AF_INET;
   stBcAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
   //stBcAddr.sin_port = htons(1238);*/

   /* bind client name to a socket */
   nBind = bind(nSocket, (struct sockaddr*)&stMyAddr, sizeof(struct sockaddr));
   if (nBind < 0)
   {
       fprintf(stderr, "%s: Can't bind a name to a socket.\n", argv[0]);
       exit(1);
   }

   /* bind client name to a socket */
   /*bcast_bind = bind(bcast_sock, (struct sockaddr*)&stMyAddr, sizeof(struct sockaddr));
   if (bcast_bind < 0)
   {
       fprintf(stderr, "%s: Can't bind a name to a socket.\n", argv[0]);
       exit(1);
   }*/

   /* send a data broadcast */
   /*strcpy(cbBuf, "yolo broadcast\n");
   ret = sendto(bcast_sock, cbBuf, strlen(cbBuf)+1, 0, (struct sockaddr*)&stBcAddr,
          sizeof(struct sockaddr));
   if (ret<0) {
        fprintf(stderr, "Error: Could not open send broadcast", argv[0]);
        close(bcast_sock);
    }*/


   for (int i=0; i < 3; i++) {
        strcpy(cbBuf, "Hello server!####\n");

       stServerAddr.sin_port = htons(ports[i]);
       sendto(nSocket, cbBuf, strlen(cbBuf)+1, 0, (struct sockaddr*)&stServerAddr,
              sizeof(struct sockaddr));

        /* receive a data from a server */
        nTmp = sizeof(struct sockaddr);
        nBytes = recvfrom(nSocket, cbBuf, BUF_SIZE, 0, (struct sockaddr*)&stServerAddr, &nTmp);
        printf("%s:: %s", argv[0], cbBuf);

        // ODCZYTAJ KLUCZ SERWERA
        int bufLen = strlen(cbBuf);
        strcpy(keyBuf, &cbBuf[bufLen-3]);
        strcat(keyBuf, &cbBuf[bufLen-2]);
        keys[i] = atoi(keyBuf);

       // connect[i] = true;
       //if (connect[i]) printf("Serwer %d jest dostępny!\n", ports[i]);
   }

   /* send a data to a server */
   printf("Podaj zdanie do zaszyfrowania: ");
   scanf("%s", cbBuf);
   strcat(cbBuf, "$$$$");
   for (int i = 0; i < 3; i++) {
     portsList[i] = ports[i];
   }
   route = make_route(portsList);
   int n = sizeof(route)/sizeof(route[0]);
   for (int i = 0; i < n; i ++) {
     sprintf(intBuf, "%d", route[n-i]);
     strcat(cbBuf, intBuf);
     printf("Przed %d szyfrowaniem:\n%s\n", i, cbBuf);
     encrypt(keys[n-1-i],cbBuf);
     printf("Po %d szyfrowaniu:\n%s\n", i, cbBuf);
   }
   strcat(cbBuf, "\n");
   stServerAddr.sin_port = htons(route[0]);
   sendto(nSocket, cbBuf, strlen(cbBuf)+1, 0, (struct sockaddr*)&stServerAddr,
          sizeof(struct sockaddr));
   printf("Wysłano wiadomość:\n%s\n", cbBuf);

   /* receive a data from a server */
   //nTmp = sizeof(struct sockaddr);
   //nBytes = recvfrom(nSocket, cbBuf, BUF_SIZE, 0, (struct sockaddr*)&stServerAddr, &nTmp);
   //printf("%s:: %s", argv[0], cbBuf);

   close(nSocket);
   //close(bcast_sock);
   return 0;
}
