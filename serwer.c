#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "encrypt.h"

#define QUEUE_SIZE 5
#define BUF_SIZE 1024

void read_server_name(char * serverName, char * bufor) {
  int bufLen = strlen(bufor);
  serverName[0] = bufor[bufLen-5];
  serverName[1] = bufor[bufLen-4];
  serverName[2] = bufor[bufLen-3];
  serverName[3] = bufor[bufLen-2];
}

int main(int argc, char* argv[])
{
   int nSocket;
   int nBind;
   int nBytes;
   int nFoo = 1, nTmp;
   int serverPort;
   srand(time(NULL));
   int key = rand() % 26;
   struct sockaddr_in stMyAddr, stClientAddr, stServerAddr;
   char cbBuf[BUF_SIZE]="", intBuf[BUF_SIZE]="", tmpBuf[BUF_SIZE]="", serverPortBuf[5]="";

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
       /* czyszczenie buforów */
       bzero(cbBuf, sizeof(cbBuf));
       bzero(intBuf, sizeof(intBuf));
       bzero(tmpBuf, sizeof(tmpBuf));
       bzero(serverPortBuf, sizeof(serverPortBuf));

       /* receive a data from a client or previous server */
       nTmp = sizeof(struct sockaddr);
       recvfrom(nSocket,cbBuf, BUF_SIZE, 0, (struct sockaddr*)&stClientAddr, &nTmp);
       printf("%s:: %s", argv[0], cbBuf);
       read_server_name(serverPortBuf, cbBuf);
       printf("serverPortBuf = %s\n", serverPortBuf);


       /* otrzymanie wiadomości od klienta i wysłanie klucza */
       if (strcmp(serverPortBuf, "####") == 0) {

         /* send a key to a client */
         sprintf(intBuf, "%d", key);
         strcpy(cbBuf, "Hello client! It's my key: ");

         if (key < 10) strcat(cbBuf, "0");
         strcat(cbBuf, intBuf);
         strcat(cbBuf, "\n");

         sendto(nSocket, cbBuf, strlen(cbBuf)+1, 0, (struct sockaddr*)&stClientAddr, nTmp);
       }
       /* otrzymanie wiadomości końcowej */
       else if (strcmp(serverPortBuf, "$$$$") == 0) {

         /* wytnij z wiadomości adres kolejnego serwera */
         strncpy(tmpBuf, cbBuf, strlen(cbBuf)-5);
         strcpy(cbBuf, tmpBuf);
         strcat(cbBuf, "\n");

         /* odczytanie wiadomości */
         printf("Jestem końcowym serwerem! Twoja wiadomość do mnie to: \n%s\n", cbBuf);
       }
       /* otrzymanie wiadomości i przesłanie jej dalej */
       else {

         /* rozszyfrowanie wiadomości */
         encrypt(-key,cbBuf);
         printf("Po rozszyfrowaniu: %s", cbBuf);

         /* odczytanie adresu kolejnego serwera */
         bzero(serverPortBuf, sizeof(serverPortBuf));
         read_server_name(serverPortBuf, cbBuf);
         serverPort = atoi(serverPortBuf);
         printf("Next server port: %d\n", serverPort);

         /* next server info */
         memset(&stServerAddr, 0, sizeof(struct sockaddr));
         stServerAddr.sin_family = AF_INET;
         stServerAddr.sin_addr.s_addr = stMyAddr.sin_addr.s_addr;
         stServerAddr.sin_port = htons(serverPort);

         /* wytnij z wiadomości adres kolejnego serwera */
         strncpy(tmpBuf, cbBuf, strlen(cbBuf)-5);
         strcpy(cbBuf, tmpBuf);
         strcat(cbBuf, "\n");

         /* send a data to a next server */
         printf("Przesyłam wiadomość:\n%s\n", cbBuf);
         sendto(nSocket, cbBuf, strlen(cbBuf)+1, 0, (struct sockaddr*)&stServerAddr, nTmp);
       }

   }

   close(nSocket);
   return(0);
}
