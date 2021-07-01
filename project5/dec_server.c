
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
} 

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[]){
  
  int connectionSocket, charsRead;
  char buffer[70000];
  char save_text[70000];
  char save_key[70000];
  char temp_char[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";    //setting up variable
  memset(save_text, '\0', sizeof(save_text));
  memset(save_key, '\0', sizeof(save_key));
  
  int convert_text;
  int convert_key;
  int convert_code;
  
  int txt_length = 0;
  int i = 0;
  int j = 0;
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);
  pid_t pid;

  // Check usage & args
  if (argc != 2) { 
    fprintf(stderr,"USAGE: %s port\n", argv[0]); 
    exit(1);
  } 
  
  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  
  if (listenSocket < 0) {
    error("ERROR opening socket");
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(listenSocket, 
          (struct sockaddr *)&serverAddress, 
          sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5); 
  
  
  // Accept a connection, blocking if one is not available until one connects
  while(1){  
  
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket, 
                (struct sockaddr *)&clientAddress, 
                &sizeOfClientInfo); 
    if (connectionSocket < 0){
      error("ERROR on accept");
    }
    
    
    //using pid to process different user in same time
    // learn from example code provide on canvas https://replit.com/@cs344/83serverc?lite=true#server.c
    //printf("%s\n", "test before into pid on enc_serves.c");
    pid = fork();
    
    
    switch(pid)
    {
      case -1:
        perror("fork()\n");
        exit(1);
        break;

      case 0:
        //printf("%s\n", "test when pid went case 0 on enc_serves.c");
        memset(buffer, '\0', sizeof(buffer));
        memset(save_text, '\0', sizeof(save_text));
        //start read from client and save it as text
        charsRead = recv(connectionSocket, save_text, sizeof(save_text), 0);
        // printf("%s\n", "test after read txt from client on enc_serves.c");
        if (charsRead < 0){
          error("ERROR reading from socket");
        }
        
        //start read from client and save it as key
        memset(buffer, '\0', sizeof(buffer));
        memset(save_key, '\0', sizeof(save_key));
        //printf("%s\n", "test after before key from client on enc_serves.c");
        charsRead = recv(connectionSocket, save_key, sizeof(save_key), 0); 
        //printf("%s\n", "test after read key from client on enc_serves.c");
        if (charsRead < 0){
          error("ERROR reading from socket");
        }        
        
        //find the length of text
        txt_length = strlen(save_text) - 1;
        //printf("%s\n", "test after strlen(save_text);");
        i = 0;
        j = 0;
        convert_text = 0;
        convert_key = 0;
        convert_code = 0;
        
        //loop from text[0] to the end of the file to record both text and key then calculate together
        while(i < txt_length){
          
          //find number for text
          for(j = 0; j < 27; j++){
            if(temp_char[j] == save_text[i]){
              convert_text = j;
            }
          }
          
          //find number for key
          for(j = 0; j < 27; j++){
            if(temp_char[j] == save_key[i]){
              convert_key = j;
              }
          } 
          
          //add both together 
          convert_code = (convert_text - convert_key) % 27;
          //if large than 26, - 27 to find new character
          
          if (convert_code < 0)
          {
          
            convert_code = convert_code + 27;
            
          }

          //simply return to the array base on the convert_code we get above
          //then save it to buffer
          buffer[i] = temp_char[convert_code];
          i++;

        }
        //printf("%s\n", "test after enc");
        //last char in buffer will be '\0'
        buffer[i] = '\0';
        //printf("%s\n", "test after add \0 ");
        
        //send process buffer to client
        charsRead = send(connectionSocket, buffer, sizeof(buffer),0); 
        if (charsRead < 0){
          error("ERROR writing to socket");
        }            
        //close connectionSocket
        close(connectionSocket); 
        exit(0);
        
   
      
      
        
      
      
    }
 } 
    
  // Close the listening socket
  close(listenSocket); 
  return 0;
}
