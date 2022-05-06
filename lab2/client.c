#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

#define MAXWAITTIME 1
#define SENDSIZE 3

int sendStuff(char *buffer, int sd, struct sockaddr_in server_address);
char *rtrim(char *s);
int main(int argc, char *argv[])
{
  int sd;//creates needed variables
  struct sockaddr_in server_address;
  char buffer[100] = "1I would like green eggs and ham. What would you like to eat today?";
  
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
  
  server_address.sin_family = AF_INET;//creates server ports and sockets
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
  int resendFrame=0;
  
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
  int a = 0;
  int done = 0;

  while(a < totalBytesToSend){ //maks sure that while there is data to send the client will keep sending data
    memset(bufferOut, 0, 100); // sets thhe memory fo the bufferr
    if(bytesLeftToSend == 1){  //checks to make sure if there are 
      sizeOfSendBuffer = 1;
      done = 1;
    }
    sprintf (bufferOut, "%11d%4d", (currentFrame),(sizeOfSendBuffer)); // prints out what you are sending

    if (bytesLeftToSend - sizeOfSendBuffer < 0){ //when there are no more things to send, exit the function
      strncpy(&bufferOut[15],&buffer[currentFrame], (bytesLeftToSend));  //loads the buffer with what is going to be needed to send
      dataSize = bytesLeftToSend;  //updates the amount of data needed to be sent
      bytesLeftToSend = 0;
      printf("NO MORE BYTES LEFT TO SEND\n");
      done = 1;
    }
    else{
      strncpy(&bufferOut[15],&buffer[currentFrame], sizeOfSendBuffer); //loads the data from the buffer to be sent
      bytesLeftToSend -= sizeOfSendBuffer;  //
      dataSize = sizeOfSendBuffer;
    }
    printf("sending '%s'\n", bufferOut);       //shows what the buffer is
    rc = sendto(sd, bufferOut, basePacketSize+dataSize, 0,(struct sockaddr *) &server_address, sizeof(server_address)); //sends message to the server
    
    if (rc < 0) // checks for the rc and if it is less tan 0 throws an error
      perror("sendto");
    printf("sent %d bytes, seqNumber %d, bottomOfWindow %d\n", rc, currentFrame, bottomOfWindow);
    if(currentFrame == bottomOfWindow){// checks to see if the message is in the right place and gives a sleep timerr
      timeSent = time(NULL);
      sleep(2);
    }
    currentFrame += dataSize; //move the current frame
    sentWindowSize += dataSize;// changes the window size to shift

    memset(bufferRead, 0, 100);
    rc = recvfrom(sd, &bufferRead, 100, MSG_DONTWAIT, (struct sockaddr *)&fromAddress, &fromLength); //lloks for the rc code of reception
  
    if(rc > 0){ //when the rc code is given updates information
      sscanf(bufferRead, "%11d", &ackNumber); //gets the ACK number
      printf ("ACK received %d \n",ackNumber);
      printf ("received data %d bytes, ack is %d, bottom/top Window %d/%d\n",rc, ackNumber, bottomOfWindow, topOfWindow);
      resendFrame = currentFrame; //keeps track of when the ACK has been received
      printf("Updated the resend Frame to %d. Current Frame is %d\n",resendFrame,currentFrame);

      if(ackNumber >= bottomOfWindow){  // Shifts the window to accomadate for the ACK
        bottomOfWindow = ackNumber + dataSize;
        sentWindowSize = currentFrame - bottomOfWindow;
        topOfWindow = bottomOfWindow + sentWindowSize;
        timeSent = time(NULL);
      }  
    }
    currentTime = time(NULL); //sets the time that the message was just sent
    
    if (currentTime - timeSent > MAXWAITTIME){ //when there is no ACK that returns, throws a timeout
      printf("\n\nTIMEOUT ****** should do a resend\n\n\n");
      int i;
      int topOfWindow = bottomOfWindow + sentWindowSize;
      printf("top of window %d, bottom %d, sentsize %d\n", topOfWindow, bottomOfWindow, sentWindowSize);
      for (i = bottomOfWindow; i < topOfWindow;){ //loops to resend the packets from when the ACK was not received.
        printf("ACK %d, bottomWindow %d\n",ackNumber, bottomOfWindow);
        memset (bufferOut, 0, 100);
      
        char *ptr;
        ptr = &buffer[i];
        printf ("str is '%s', length is %lu\n",ptr, strlen(ptr));

        if(bytesLeftToSend ==1){
          sprintf(bufferOut, "%11d%4d", (i),1);
        }
	else{
	  sprintf (bufferOut, "%11d%4d", (resendFrame),(sizeOfSendBuffer));//loads the data into buffer
	}
	
        strncpy(&bufferOut[15],&buffer[resendFrame], sizeOfSendBuffer); //Data
        dataSize = sizeOfSendBuffer;
	printf("resending '%s'\n", bufferOut);
      
        
        rc = sendto(sd, bufferOut, basePacketSize+dataSize, 0,(struct sockaddr *) &server_address, sizeof(server_address)); 
        
        i += 2;
      }
      bottomOfWindow = currentFrame;//resets the bottom of window to match
      resendFrame = currentFrame;
    }
    if (done == 1){
      a = totalBytesToSend + 1;
    }
    a = a + 1;
  }
  return 0;
}
