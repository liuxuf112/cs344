
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <fcntl.h>

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char *msg) { 
  perror(msg); 
  exit(0); 
} 

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber, 
                        char* hostname){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname(hostname); 
  if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

int main(int argc, char *argv[]) {

  //printf("%s\n", "test1 on enc_client.c");
  



  //check the length of key is not smaller than txt
  
  /*
  FILE * txt_fp = fopen(argv[1],"r");
  if(!txt_fp) return -1;
  int txt_size = fseek(txt_fp,0,SEEK_END);
  fclose(txt_fp);

  FILE * key_fp = fopen(argv[2],"r");
  if(!key_fp) return -1;
  int key_size = fseek(key_fp,0,SEEK_END);
  fclose(key_fp);
  
  
  */
  //learn from this website about using leseek to find size of the file https://blog.csdn.net/wr_iskye/article/details/83418135
  //printf("%s\n", "test7 on enc_client.c");
  
  int txt = open(argv[1], O_RDONLY);
  int txt_size = lseek(txt, 0, SEEK_END);
  int key = open(argv[2], O_RDONLY);
  int key_size = lseek(key, 0, SEEK_END);
  
  //printf("%s\n", "test8 on enc_client.c");
  
  //printf(" size of key is %d \n", key_size); //%s, segfault
  //printf(" size of txt is %d \n", txt_size);
  
  
  if(key_size < txt_size){
    error("CLIENT: Key size smaller than txt size");
    exit(3); 
  }
  
  
  int socketFD, portNumber, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  char buffer[70000];
  // Check usage & args
  if (argc < 4) { 
    fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); 
    exit(0); 
  } 

  // Create a socket
   //printf("%s\n", "test2 on enc_client.c");
  socketFD = socket(AF_INET, SOCK_STREAM, 0); 
  if (socketFD < 0){
    error("CLIENT: ERROR opening socket");
  }

   // Set up the server address struct
    //("%s\n", "test3 on enc_client.c");
  setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");
  
  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("CLIENT: ERROR connecting");
  }
  
  
  
  
  
  
  //printf("%s\n", "test4 on enc_client.c");
  // learn reading file from txt ,https://www.educative.io/edpresso/c-reading-data-from-a-file-using-fread
  // Get input message from file, first txt then key
  // open txt file the fread txt, then send to server
  FILE * txt_file = fopen(argv[1],"r");
  // Clear out the buffer array
  memset(buffer, '\0', sizeof(buffer));
  txt_size = fread(buffer, sizeof(char), sizeof(buffer), txt_file);
  
  // Send message to server
  // Write to the server
  charsWritten = send(socketFD, buffer, sizeof(buffer), 0); 
  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }
  if (charsWritten < strlen(buffer)){
    //error("CLIENT: WARNING: Not all data written to socket!\n");
  }
  fclose(txt_file);


  //printf("%s\n", "test5 on enc_client.c");
  //start key file
  


  FILE * key_file = fopen(argv[2],"r");
  // Clear out the buffer array
  memset(buffer, '\0', sizeof(buffer));
  key_size = fread(buffer, sizeof(char), sizeof(buffer), key_file);
  
  // Send message to server
  // Write to the server
  charsWritten = send(socketFD, buffer, sizeof(buffer), 0);  
  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }
  if (charsWritten < strlen(buffer)){
    //printf("CLIENT: WARNING: Not all data written to socket!\n");
  }
  fclose(key_file);
  
    //printf("%s\n", "test6 on enc_client.c");

  

  // Get return message from server
  // Clear out the buffer again for reuse
  memset(buffer, '\0', sizeof(buffer));
  // Read data from the socket, leaving \0 at end
  charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
  
  
  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket");
  }
  printf("%s\n", buffer);

  // Close the socket
  close(socketFD); 

  return 0;
}