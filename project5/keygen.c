#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


int main(int argc, char* argv[]){

  srand(time(NULL)); 

  if (argc != 2){                            //simple check if the input argument is correct or not, if not, exit
    fprintf(stderr, "Error Input\n");
    exit(3);
  
  }
  
  
  int length;
  length = atoi(argv[1]);
  int i;

  char temp[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  
  for(i = 0; i < length; i++){
  
    printf("%c", temp[random() %27]);
    
  } 

  
  printf("\n");
  



  return 0;
  
}
