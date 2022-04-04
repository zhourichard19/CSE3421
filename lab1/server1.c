/*Richard Zhou*/

#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]){
  int sd;/*socket descriptor*/
  int rc;/* return code from recvfrom*/
  struct sockaddr_in server_address; /*address of this serverr*/
  struct sockaddr_in from_address;/*address filled in on recv*/
  char buffer[100];/*where i will put data*/
  int flags = 0;/*used in socket calls*/
  socklen_t fromLength = sizeof (struct sockaddr);/*parm in socket calls*/

  if(argc < 2){ // program expects at least 2 parameters
      printf("usage is: server <portnumber>\n");
      exit(1);
  }

  sd = socket(AF_INET, SOCK_DGRAM, 0); // create a socket

  /*now fix/set the address of the server*/
  int portNumber = atoi(argv[1]);
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(portNumber);
  server_address.sin_addr.s_addr = INADDR_ANY;// special *****
  
  /*once the address structure is filled in, pass to the OS/Network */
  rc = bind(sd, (struct sockaddr *)&server_address, sizeof(server_address));
  if(rc < 0){// ALWAYS check the rc
      perror("bind");
      exit(1);
  }

  printf("outside");

  // Loop foreverr and get data from clients
  for(;;){
    printf("here");
    memset(buffer,0,100);
    printf("there");
    rc = recvfrom(sd, buffer, sizeof(buffer), flags, (struct sockaddr *)&from_address, &fromLength);
    if(rc <=0){
        printf("some kind of error on recvfrom(). Going to quit\n");
        break;
    }
    printf("I received %d bytes \n", rc);
  }
    
  return 0;
}
