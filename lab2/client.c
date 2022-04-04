#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

#define WINDOWSIZE 20
#define MAXWAITTIME 1
#define SENDSIZE 3

int sendStuff(char *buffer, int sd, struct sockaddr_in server_address);
char *rtrim(char *s);
int main(int argc, char *argv[])
{
  int sd;
  struct sockaddr_in server_address;
  char buffer[100] = "I would like green eggs and ham. What would you like to eat today?";
  
  int portNumber;
  char serverIP[29];
  

  if (argc < 3){
    printf ("usage is client <ipaddr> <portnumber>\n");
    exit(1);
  }
  
  printf ("you are sending '%s'\n", buffer);
  
  sd = socket(AF_INET, SOCK_DGRAM, 0);
    

  portNumber = strtol(argv[2], NULL, 10);
  strcpy(serverIP, argv[1]);
  
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(portNumber);
  server_address.sin_addr.s_addr = inet_addr(serverIP);
  sendStuff(buffer, sd, server_address);
  

  return 0 ; 
}

int sendStuff(char *buffer, int sd, struct sockaddr_in server_address){
  int bottomOfWindow, topOfWindow;
  bottomOfWindow = topOfWindow = 0;
  time_t timeSent;
  time_t currentTime;
  int currentFrame;
  int rc;
  int maxWindow = WINDOWSIZE;
  char bufferOut[100];
  struct sockaddr_in fromAddress;
  socklen_t fromLength = sizeof(struct sockaddr_in);
  char bufferRead[100];
  int ackNumber = 0; 
  int totalBytesToSend;
  int bytesLeftToSend;
  int sizeOfSendBuffer = 2;
  int sentWindowSize = 0;
  int dataSize = sizeOfSendBuffer;
  int basePacketSize = 15;
  
  totalBytesToSend = strlen(buffer); //change this once we add in file IO
  printf ("totalBytesToSend %d\n", totalBytesToSend);
  bytesLeftToSend = totalBytesToSend; 
  // handle sending the filename and filesize differently
  //  sprintf (bufferOut, "%11d",  htons(totalBytesToSend));
  totalBytesToSend = htonl(totalBytesToSend);
  rc = sendto(sd, &totalBytesToSend, sizeof(totalBytesToSend), 0,
  (struct sockaddr *) &server_address, sizeof(server_address));
  totalBytesToSend = ntohl(totalBytesToSend);
  currentFrame = 0;
  sizeOfSendBuffer = 2; // will change potentially for last frame
  sprintf (bufferOut, "%11d%4d", (currentFrame),(sizeOfSendBuffer));
  
  if (bytesLeftToSend - sizeOfSendBuffer <0){
    strncpy(&bufferOut[15],&buffer[currentFrame], (bytesLeftToSend));
    dataSize = bytesLeftToSend;
    bytesLeftToSend = 0;
  }
  else{
    strncpy(&bufferOut[15],&buffer[currentFrame], sizeOfSendBuffer); //Data
    bytesLeftToSend -= sizeOfSendBuffer;
    dataSize = sizeOfSendBuffer;
      
    printf("sending '%s'\n", bufferOut);       
    rc = sendto(sd, bufferOut, basePacketSize+dataSize, 0,(struct sockaddr *) &server_address, sizeof(server_address));   
    if (rc < 0)
      perror("sendto");
    printf("sent %d bytes, seqNumber %d, bottomOfWindow %d\n", rc, currentFrame, bottomOfWindow);
    if(currentFrame == bottomOfWindow){
      timeSent = time(NULL);
    }
    currentFrame += dataSize;
    sentWindowSize += dataSize;
}// end of the if there is room to sne dstuff statment

  memset(bufferRead, 0, 100);
  rc = recvfrom(sd, &bufferRead, 100, MSG_DONTWAIT, (struct sockaddr *)&fromAddress, &fromLength);
  
  if(rc > 0){
    sscanf(bufferRead, "%11d", &ackNumber);
    printf ("ACK received %d \n",ackNumber);
    printf ("received data %d bytes, ack is %d, bottom/top Window %d/%d\n",rc, ackNumber, bottomOfWindow, topOfWindow);

    if(ackNumber >= bottomOfWindow){
      bottomOfWindow = ackNumber + dataSize;
      sentWindowSize = currentFrame - bottomOfWindow;
      topOfWindow = bottomOfWindow + sentWindowSize;
      timeSent = time(NULL);
    }  
  }
  currentTime = time(NULL);
  if (currentTime - timeSent > MAXWAITTIME){
    printf("\n\nTIMEOUT ****** should do a resend\n\n\n");
    int i;
    int topOfWindow = bottomOfWindow + sentWindowSize;
    printf("top of window %d, bottom %d, sentsize %d\n", topOfWindow, bottomOfWindow, sentWindowSize);
    for (i = bottomOfWindow; i < topOfWindow;){
      memset (bufferOut, 0, 100);
      
      char *ptr;
      ptr = &buffer[i];
      printf ("str is '%s', length is %lu\n",ptr, strlen(ptr));

      if(strlen(ptr) ==1){
        sprintf(bufferOut, "%11d%4d", (i),1);
      }
    } 
  }
  return 0;
}