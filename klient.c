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
#define PORT_NUMBER 1234


int main(int argc, char* argv[])
{
   int nSocket, bcSocket;
   int nConnect;
   int nBytes;
   int nBind, bcBind;
   int nTmp;
   struct sockaddr_in stServerAddr, stMyAddr, stBcAddr;
   struct hostent* lpstServerEnt, destServerEnt;
   char cbBuf[BUF_SIZE], keyBuf[BUF_SIZE], intBuf[BUF_SIZE];
   srand(time(NULL));
   //int ports[] = {1230, 1231, 1232};
   //int keys[] = {0};
   char addresses[MAX_SERVER_NR][INET_ADDRSTRLEN] = {{0}};
   int addressesLenght[MAX_SERVER_NR];
   char destAddress[INET_ADDRSTRLEN] = "";
   int keys[MAX_SERVER_NR] = {0};

   if (argc != 2)
   {
     fprintf(stderr, "Usage: %s server_name\n", argv[0]);
     exit(1);
   }

   /* look up server's IP address */
   destServerEnt = gethostbyname(argv[1]);
   strcpy(destAddress, argv[1]);
   if (! lpstServerEnt)
   {
      fprintf(stderr, "%s: Can't get the destination server's IP address.\n", argv[0]);
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
   //memset(&stServerAddr, 0, sizeof(struct sockaddr));
   //stServerAddr.sin_family = AF_INET;
   //memcpy(&stServerAddr.sin_addr.s_addr, lpstServerEnt->h_addr, lpstServerEnt->h_length);

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
   stBcAddr.sin_port = htons(PORT_NUMBER);
   stBcAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

   strcpy(cbBuf, "Hello broadcast!##\n");

   /* wysłanie wiadomości serwerom */
   sendto(bcSocket, cbBuf, strlen(cbBuf)+1, 0, (struct sockaddr*)&stBcAddr,
          sizeof(struct sockaddr));

    /* ograniczenie czasu na otrzymywanie wiadomości od serwerów */
    time_t initial_time = time(NULL);
    float time_limit = 1.0f;
    struct timeval tv;
    tv.tv_sec = 0;  /* 0.2 Secs Timeout */
    tv.tv_usec = 200;
    setsockopt(bcSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));
    int servers = 0;

    /* wyłaczenie pozwolenia na wysłanie broadcastu */
    broadcastEnable=0;
    setsockopt(bcSocket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

    /* znajdź serwery, odczytaj klucze i adresy*/
    while ( (time(NULL) - initial_time) < time_limit || servers > MAX_SERVER_NR)
    {
      /* czyszczenie buforów */
      bzero(cbBuf, sizeof(cbBuf));
      bzero(keyBuf, sizeof(keyBuf));

       /* receive a data from a server */
       nTmp = sizeof(struct sockaddr);
       nBytes = recvfrom(bcSocket, cbBuf, BUF_SIZE, 0, (struct sockaddr*)&stBcAddr, &nTmp);
       /*jeżeli otrzymano wiadomość */
       if (nBytes != -1) {
         /* odczytanie adresu serwera */
         char ipAddress[INET_ADDRSTRLEN];
         inet_ntop(AF_INET, &(stBcAddr.sin_addr), ipAddress, INET_ADDRSTRLEN);
         /* jeżeli nie jest adresem serwera końcowego */
         if (strcmp(ipAddress, destAddress) == 0) {

             /* odczytanie otrzymanego klucza */
             int bufLen = strlen(cbBuf);
             strcpy(keyBuf, &cbBuf[bufLen-3]);
             strcat(keyBuf, &cbBuf[bufLen-2]);
             keys[servers] = atoi(keyBuf);

             /* zapisanie adresu serwera */
             strcpy(addresses[servers],ipAddress);
             addressesLenght[servers] = strlen(ipAddress);

             printf("Klucz: %d, długość adresu: %d, adres: %s\n", keys[servers], addressesLenght[servers], addresses[servers]);
             printf("%s:: %s", argv[0], cbBuf);
             servers++;
         }
       }
    }


   //BROADCAST
   /* wylosowanie kolejności serwerów */
   int route[servers];
   memset( route, 0, servers*sizeof(int) );
   make_route(route, servers);

   /* stworzenie wylosowanej listy serwerów z kluczami */
   char addressesList[MAX_SERVER_NR][INET_ADDRSTRLEN] = {{0}};
   int keysList[MAX_SERVER_NR];
   int addressesLenghtList[MAX_SERVER_NR];
   memset( keysList, 0, servers*sizeof(int) );
   memset( addressesLenghtList, 0, servers*sizeof(int) );
   for(int i = 0; i < servers; i++) {
     strcpy(addressesList[i], addresses[route[i]]);
     keysList[i] = keys[route[i]];
     addressesLenghtList[i] = addressesLenght[route[i]];
   }

   /* czyszczenie buforów */
   bzero(cbBuf, sizeof(cbBuf));
   bzero(intBuf, sizeof(intBuf));

   /* odczytanie danych do wysłania od użytkownika */
   printf("Podaj zdanie do zaszyfrowania: ");
   scanf("%s", cbBuf);

   /* dopisanie znaków sterujących */
   strcat(cbBuf, "$$");

   /* zaszyfrowanie wiadomości i dopisanie kolejności serwerów */
   for (int i = 0; i < servers; i ++) {
     if (i == 0) {
       /* dopisz IP serwera końcowego */
       strcat(cbBuf, destAddress);
       /* dopisz liczbę jego znaków */
       if (strlen(destAddress) < 10) strcat(cbBuf, "0");
       sprintf(intBuf, "%d", strlen(destAddress));
       strcat(cbBuf, intBuf);
     }
     if (i > 0) {
       /* dopisz IP serwera */
       strcat(cbBuf, addressesList[servers-i]);
       /* dopisz liczbę jego znaków */
       if (addressesLenghtList[servers-i] < 10) strcat(cbBuf, "0");
       sprintf(intBuf, "%d", addressesLenghtList[i]);
       strcat(cbBuf, intBuf);
     }
     printf("Przed %d szyfrowaniem:\n%s\n", i, cbBuf);
     encrypt(keysList[servers-1-i],cbBuf);
     printf("Po %d szyfrowaniu szyfrem %d:\n%s\n", i, keysList[servers-1-i], cbBuf);
     bzero(intBuf, sizeof(intBuf));
   }
   strcat(cbBuf, "\n");

   /* send a data to a server */
   lpstServerEnt = gethostbyname(addressesList[0]);
   memcpy(&stBcAddr.sin_addr.s_addr, lpstServerEnt->h_addr, lpstServerEnt->h_length);
   sendto(nSocket, cbBuf, strlen(cbBuf)+1, 0, (struct sockaddr*)&stBcAddr,
          sizeof(struct sockaddr));
   printf("Wysłano wiadomość:\n%s\n", cbBuf);


   //STARA WERSJA Z WYSYŁANIEM NA RÓŻNE PORTY PRZY TYM SAMYM ADRESIE IP

      /* znajdź serwery i odczytaj klucze */
      //for (int i=0; i < 3; i++) {

          /* czyszczenie buforów */
          //bzero(cbBuf, sizeof(cbBuf));
          //bzero(keyBuf, sizeof(keyBuf));

          //strcpy(cbBuf, "Hello server!####\n");

          /* wysłanie wiadomości serwerom */
          //stServerAddr.sin_port = htons(ports[i]);
          //sendto(nSocket, cbBuf, strlen(cbBuf)+1, 0, (struct sockaddr*)&stServerAddr,
         //        sizeof(struct sockaddr));

           /* receive a data from a server */
           //nTmp = sizeof(struct sockaddr);
           //nBytes = recvfrom(nSocket, cbBuf, BUF_SIZE, 0, (struct sockaddr*)&stServerAddr, &nTmp);
           //printf("%s:: %s", argv[0], cbBuf);

           /* odczytanie otrzymanego klucza */
           //int bufLen = strlen(cbBuf);
           //strcpy(keyBuf, &cbBuf[bufLen-3]);
           //strcat(keyBuf, &cbBuf[bufLen-2]);
           //keys[i] = atoi(keyBuf);
      //}




      /* czyszczenie buforów */
      //bzero(cbBuf, sizeof(cbBuf));
      //bzero(intBuf, sizeof(intBuf));

      /* odczytanie danych do wysłania od użytkownika */
      //printf("Podaj zdanie do zaszyfrowania: ");
      //scanf("%s", cbBuf);

      /* dopisanie znaków sterujących */
      //strcat(cbBuf, "$$$$");



      /* stworzenie wylosowanej listy serwerów z kluczami */
      //int portsList[MAX_SERVER_NR], keysList[MAX_SERVER_NR];
      //memset( portsList, 0, servers*sizeof(int) );
      //memset( keysList, 0, servers*sizeof(int) );
      //for(int i = 0; i < servers; i++) {
     //   portsList[i] = ports[route[i]];
     //   keysList[i] = keys[route[i]];
      //}




      /* zaszyfrowanie wiadomości i dopisanie kolejności serwerów */
      //for (int i = 0; i < servers; i ++) {
     //   if (i > 0) {
     //     sprintf(intBuf, "%d", portsList[servers-i]);
     //     strcat(cbBuf, intBuf);
     //   }
     //   printf("Przed %d szyfrowaniem:\n%s\n", i, cbBuf);
     //   encrypt(keysList[servers-1-i],cbBuf);
     //   printf("Po %d szyfrowaniu:\n%s\n", i, cbBuf);
      //}
      //strcat(cbBuf, "\n");

      /* send a data to a server */
      //stServerAddr.sin_port = htons(portsList[0]);
      //sendto(nSocket, cbBuf, strlen(cbBuf)+1, 0, (struct sockaddr*)&stServerAddr,
     //        sizeof(struct sockaddr));
      //printf("Wysłano wiadomość:\n%s\n", cbBuf);





   close(nSocket);
   close(bcSocket);
   return 0;
}
