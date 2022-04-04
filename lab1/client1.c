/*Richard Zhou*/

#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

// derfine some functions
int sendStuff(char *buffer, int sd, struct sockaddr_in server_address);

int main(int argc, char *argv[]){
  int sd;/*socket descriptor*/
  struct sockaddr_in server_address; /* address of Server*/
  int portNumber;/*port number for server*/
  char serverIP[29]; /* IP of server */
  char buffer [100];/* where i put data*/

  if (argc < 3){ /*error, exit*/
      printf("usage is in client <ipaddr> <portnumber>\n");
      exit(1);
  }

  /*step1, create the socket*/
  sd = socket(AF_INET, SOCK_DGRAM, 0);

  /*now get the ip and port from the command line*/
  portNumber = strtol(argv[2], NULL, 10);
  strcpy(serverIP, argv[1]);

  /*set up the structure containing the address of server*/
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(portNumber);
  server_address.sin_addr.s_addr = inet_addr(serverIP);

  memset(buffer, '\0', 100);
  sprintf(buffer, "hello world");
  printf("you are sending '%s'\n", buffer);  
  printf("the length of the string is %lu bytes\n", strlen(buffer));

  /*handle the send in a function*/
  sendStuff(buffer, sd, server_address);
  close(sd);
  return 0;
}

int sendStuff(char*buffer, int sd, struct sockaddr_in server_address){
  /*this functioni handles sendiing data to a server*/
  int rc = 0;
  rc = sendto(sd, buffer, strlen(buffer), 0,(struct sockaddr *)&server_address, sizeof(server_address));

  if(rc <=0){
      printf("error, didn't send any bytes");
      exit(1);
  }
  printf("I think i sent %d bytes \n",rc);
  return 0;
}
