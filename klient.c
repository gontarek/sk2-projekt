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
#define MAX_SERVER_NR 10



int main(int argc, char* argv[])
{
   int nSocket, bcSocket;
   int nConnect;
   int nBytes;
   int nBind, bcBind;
   int nTmp;
   struct sockaddr_in stServerAddr, stMyAddr, stBcAddr;
   struct hostent* lpstServerEnt;
   char cbBuf[BUF_SIZE], keyBuf[BUF_SIZE], intBuf[BUF_SIZE];
   srand(time(NULL));
   int ports[] = {1230, 1231, 1232};
   int keys[] = {0};
   struct sockaddr_in addresses[MAX_SERVER_NR] = {{0}};
   int keys2[MAX_SERVER_NR] = {0};

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

   /* create a socket */
   bcSocket = socket(PF_INET, SOCK_DGRAM, 0);
   if (bcSocket < 0)
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

   /* bind client name to a socket */
   nBind = bind(nSocket, (struct sockaddr*)&stMyAddr, sizeof(struct sockaddr));
   if (nBind < 0)
   {
       fprintf(stderr, "%s: Can't bind a name to a socket.\n", argv[0]);
       exit(1);
   }

   /* bind client name to a socket */
   bcBind = bind(bcSocket, (struct sockaddr*)&stMyAddr, sizeof(struct sockaddr));
   if (bcBind < 0)
   {
       fprintf(stderr, "%s: Can't bind a name to a socket.\n", argv[0]);
       exit(1);
   }

   /* czyszczenie buforów */
   bzero(cbBuf, sizeof(cbBuf));
   bzero(keyBuf, sizeof(keyBuf));

   /* pozwolenie na wysłanie broadcastu */
   int broadcastEnable=1;
   setsockopt(bcSocket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

   /* struktura adresu broadcastowego serwera */
   memset(&stBcAddr, 0, sizeof(struct sockaddr));
   stBcAddr.sin_family = AF_INET;
   stBcAddr.sin_port = htons(1234);
   stBcAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

   strcpy(cbBuf, "Hello broadcast!####\n");

   /* wysłanie wiadomości serwerom */
   sendto(bcSocket, cbBuf, strlen(cbBuf)+1, 0, (struct sockaddr*)&stBcAddr,
          sizeof(struct sockaddr));

    /* ograniczenie czasu na otrzymywanie wiadomości od serwerów */
    time_t initial_time = time(NULL);
    float time_limit = 1.0f;
    struct timeval tv;
    tv.tv_sec = 0;  /* 0.1 Secs Timeout */
    tv.tv_usec = 100;
    setsockopt(bcSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));
    int servers = 0;

    /* wyłaczenie pozwolenia na wysłanie broadcastu */
    broadcastEnable=0;
    setsockopt(bcSocket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

    while ( (time(NULL) - initial_time) < time_limit || servers > MAX_SERVER_NR)
    {
      /* czyszczenie buforów */
      bzero(cbBuf, sizeof(cbBuf));
      bzero(keyBuf, sizeof(keyBuf));

      //printf("Przed otrzymaniem\n");
       /* receive a data from a server */
       nTmp = sizeof(struct sockaddr);
       nBytes = recvfrom(bcSocket, cbBuf, BUF_SIZE, 0, (struct sockaddr*)&stBcAddr, &nTmp);
       if (nBytes != -1) {
         /* odczytanie otrzymanego klucza */
         int bufLen = strlen(cbBuf);
         strcpy(keyBuf, &cbBuf[bufLen-3]);
         strcat(keyBuf, &cbBuf[bufLen-2]);
         keys2[servers] = atoi(keyBuf);
         /* odczytanie adresu serwera */
         addresses[servers] = stBcAddr;
         printf("Klucz: %d, adres: %d\n", keys2[servers], addresses[servers]);
         printf("%s:: %s", argv[0], cbBuf);
         servers++;
       }

       //printf("Po otrzymanie %d bajtów\n", nBytes);
    }



   /* znajdź serwery i odczytaj klucze */
   for (int i=0; i < 3; i++) {

       /* czyszczenie buforów */
       bzero(cbBuf, sizeof(cbBuf));
       bzero(keyBuf, sizeof(keyBuf));

       strcpy(cbBuf, "Hello server!####\n");

       /* wysłanie wiadomości serwerom */
       stServerAddr.sin_port = htons(ports[i]);
       sendto(nSocket, cbBuf, strlen(cbBuf)+1, 0, (struct sockaddr*)&stServerAddr,
              sizeof(struct sockaddr));

        /* receive a data from a server */
        nTmp = sizeof(struct sockaddr);
        nBytes = recvfrom(nSocket, cbBuf, BUF_SIZE, 0, (struct sockaddr*)&stServerAddr, &nTmp);
        printf("%s:: %s", argv[0], cbBuf);

        /* odczytanie otrzymanego klucza */
        int bufLen = strlen(cbBuf);
        strcpy(keyBuf, &cbBuf[bufLen-3]);
        strcat(keyBuf, &cbBuf[bufLen-2]);
        keys[i] = atoi(keyBuf);
   }




   /* czyszczenie buforów */
   bzero(cbBuf, sizeof(cbBuf));
   bzero(intBuf, sizeof(intBuf));

   /* odczytanie danych do wysłania od użytkownika */
   printf("Podaj zdanie do zaszyfrowania: ");
   scanf("%s", cbBuf);

   /* dopisanie znaków sterujących */
   strcat(cbBuf, "$$$$");

   /* wylosowanie kolejności serwerów */
   int serversNumber = 0;
   serversNumber = sizeof(ports)/sizeof(ports[0]);
   int route[serversNumber];
   memset( route, 0, serversNumber*sizeof(int) );
   int n = sizeof(route)/sizeof(route[0]);
   make_route(route, n);

   /* stworzenie wylosowanej listy serwerów z kluczami */
   int portsList[serversNumber], keysList[serversNumber];
   memset( portsList, 0, serversNumber*sizeof(int) );
   memset( keysList, 0, serversNumber*sizeof(int) );
   for(int i = 0; i < n; i++) {
     portsList[i] = ports[route[i]];
     keysList[i] = keys[route[i]];
   }

   /* zaszyfrowanie wiadomości i dopisanie kolejności serwerów */
   for (int i = 0; i < n; i ++) {
     if (i > 0) {
       sprintf(intBuf, "%d", portsList[n-i]);
       strcat(cbBuf, intBuf);
     }
     printf("Przed %d szyfrowaniem:\n%s\n", i, cbBuf);
     encrypt(keysList[n-1-i],cbBuf);
     printf("Po %d szyfrowaniu:\n%s\n", i, cbBuf);
   }
   strcat(cbBuf, "\n");

   /* send a data to a server */
   stServerAddr.sin_port = htons(portsList[0]);
   sendto(nSocket, cbBuf, strlen(cbBuf)+1, 0, (struct sockaddr*)&stServerAddr,
          sizeof(struct sockaddr));
   printf("Wysłano wiadomość:\n%s\n", cbBuf);


   close(nSocket);
   return 0;
}
