
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
/*

  xufeng Liu
  5/3/2021

*/
int foreground_only = 0;      // this foreground_only is globle var, and it use to keep tracking forground mode

void generate_argument(char user_input[2048], char * input_argument[512], int * arugment_index,char * input_file, char * output_file, int * background,int* input_index, int * output_index, int * background_index){

  // learn from https://www.geeksforgeeks.org/strtok-strtok_r-functions-c-examples/
  // https://stackoverflow.com/questions/3889992/how-does-strtok-split-the-string-into-tokens-in-c
  // https://www.youtube.com/watch?v=a8l8PwCzw20
  
  char * token;                                  // set a token keep receving from string
  int i = 0;
  token = strtok(user_input, " \n");             // this part will make a string to different piceses, and it will save in the the 2d input argument array. Also the the token keep the first.
  
  while(token != NULL)                           // the string is not finish or not touch to NULL it will keep running
  {

   if(strcmp(token,"<") == 0){                   // if detect <, means the next will be a input file, save the index and file name
   
     //printf("\naaaaaaaaaaaaaaa\n");
     token = strtok(NULL, " \n"); 
     //input_file = token;
     strcpy(input_file,token);  
     *input_index = i;                          //save the index number in argument

   }
   
   if(strcmp(token,">") == 0){                  //same method like <, but simply change the checking from < to >.
     
     //printf("\nbbbbbbbbbbbbbbbbbb\n");
     token = strtok(NULL, " \n"); 
     //output_file = token;
     strcpy(output_file,token);
     *output_index = i;
 
   }
   
   if(strcmp(token,"&") == 0){                   // if user input with &, means it will go backgound mode, set the index and pass 1 into backgound, 1 means it on.
     
      *background = 1;
      *background_index = i;
      
      
   }
   

    //printf("\n i is %d\n", i);
    //printf("\n token is %s\n", token);
    
    input_argument[i] =  token;                //the data we get from token will keep sending to input_argument, so save into the 2d array.
   
    token = NULL;
    //printf("\n input_argument is %s\n", input_argument[i]); 
 
    i++;                                        //i++ for moving next argument
    token = strtok(NULL, " \n");

  }
  input_argument[i] =  NULL;                    //After data generated to the argument, we will set NULL to the end of the argument.
  *arugment_index = i;                          // record the total length of argument
  
  
}

/*

test is use to run foreground

*/

void test(char * input_argument[512],int * arugment_index,char * input_file, char * output_file, int * background,int* input_index, int * output_index, int * background_index, int * childStatus_result){
  
  
  //printf("\n arugment_index is %d\n", *arugment_index);
  /*
  int s;
  int test_index = *arugment_index;
  for(s = 0; s < test_index; s++){
    printf("\n input_argument in %d is %s\n", s, input_argument[s]);
  }
  
  */
  //printf("\nRun base on foreground-only mode (& is now ignored)\n");
  fflush(stdout);
  int childStatus;
  pid_t spawnPid = fork();
  
  int result;
  int sourceFD;
  int targetFD;
    
  switch(spawnPid){                                //same thing from canvas but add a little bit change. https://repl.it/@cs344/54sortViaFilesc
    
    case -1:
      perror("fork()\n");
      exit(1);
      break;
      
    case 0:
    
    
      
      if(*input_index != 0){
        
        sourceFD = open(input_file, O_RDONLY);              // if find a input file from user, it will that direct the file from user input
        if (sourceFD == -1) {
          perror(" Error open file");     
          exit(1);
        }
        printf("sourceFD == %d\n", sourceFD);      
        fflush(stdout);
        
        // Redirect stdin to source file
        result = dup2(sourceFD, 0);    
        input_argument[*input_index] = NULL;                 //after dup2 the file, we will set the argument base on the Index we record and set it to NULL, it means we will not see the text for input file in the exec()
        if (result == -1) { 
          perror("source dup2()"); 
          exit(2);
        }
        
        fcntl(sourceFD, F_SETFD, FD_CLOEXEC);
      }
      
      
      
      
      if(*output_index != 0){                              // it most same as the input, but we change the method of open a file.
        
        //printf("\n  outputfile is %s\n", output_file);
        targetFD = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);  //so the open file method going to be 0644, it means it will create one if there have none
        
        if (targetFD == -1) {
          perror("Error open file");
          exit(1);      
        }
        printf("The file descriptor for targetFD is %d\n", targetFD);    //geting result by dup2 and redirect into arguments
        fflush(stdout);
        
        result = dup2(targetFD, 1);
        input_argument[*output_index] = NULL;
        if (result == -1) {                                         //if there have any error, using perror to warning and exit(2)
          perror("target dup2()"); 
          exit(2);
        } 
        fcntl(targetFD, F_SETFD, FD_CLOEXEC);
       }
      
      
      if(*background!= 0){
      
        input_argument[*background_index] = NULL;                    //if it shows that the command is with &, save the array index so we will remove it later.

      }
  
      //printf("\n input_argument[0]is %s\n", input_argument[0]);
      //printf("\n input_argument[1] is %s \n", input_argument[1]);
      //printf("\n input_argument[2] is %s \n", input_argument[2]);
      execvp(input_argument[0], input_argument);
      perror("execvp"); 
      exit(2);
      break;
    
    default:
      spawnPid = waitpid(spawnPid, &childStatus,0);                                  // it is forground, so we will have a waitpid to wait the child process.
      

      if(WIFEXITED(childStatus)){                                                    //save the child result.
            //printf("\n %d\n",WEXITSTATUS(childStatus));
            fflush(stdout);
           *childStatus_result = WEXITSTATUS(childStatus);
        }
      else{
          //printf("\n %d\n",WTERMSIG(childStatus));
          fflush(stdout);
          *childStatus_result = WTERMSIG(childStatus);
        }
        
        
        
      while((spawnPid = waitpid(-1, &childStatus, WNOHANG)) > 0){                          // this part will keep detecting if there are any background process is running, and if one is done, print result
      
      if(WIFEXITED(childStatus)){
  
      fflush(stdout);
      printf("background pid %d is done: exit value %d\n", spawnPid, WEXITSTATUS(childStatus));
      fflush(stdout);
      
      }
      else{
      printf("background pid %d is done: terminated by signal %d\n", spawnPid, WTERMSIG(childStatus));
      fflush(stdout);
      
      }
      
      }
        
      
      break;
  
  
  }
  

}


/*
  test2 is run as the background


*/


void test2(char * input_argument[512],int * arugment_index,char * input_file, char * output_file, int * background,int* input_index, int * output_index, int * background_index, int * childStatus_result){
  

  fflush(stdout);
  int childStatus;                                          //test2 is most same as test1. the only different is in default there no waitpid() , and also display the pid for child process.
  pid_t spawnPid = fork();
  
  int result;
  int sourceFD;
  int targetFD;
  
  switch(spawnPid){
  
    case -1:
      perror("fork()\n");
      exit(1);
      break;
      
    case 0:
    
    // using and learn from canvas https://repl.it/@cs344/54sortViaFilesc
      
      if(*input_index != 0){                                                            //if there are input, start collect the input
        
        sourceFD = open(input_file, O_RDONLY);
        if (sourceFD == -1) {
          perror(" Error open file");     
          exit(1);
        }
        printf("sourceFD == %d\n", sourceFD);  
        fflush(stdout);
        
        // Redirect stdin to source file
        result = dup2(sourceFD, 0);    
        input_argument[*input_index] = NULL; 
        if (result == -1) { 
          perror("source dup2()"); 
          exit(2);
        }
        
        fcntl(sourceFD, F_SETFD, FD_CLOEXEC);
      }
          
      if(*output_index != 0){                                                          //if there are output requirement, start direct to the output
        
        //printf("\n  outputfile is %s\n", output_file);
        targetFD = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        
        if (targetFD == -1) {
          perror("Error open file");
          exit(1);      
        }
        printf("The file descriptor for targetFD is %d\n", targetFD);
        fflush(stdout);
        
        result = dup2(targetFD, 1);
        input_argument[*output_index] = NULL;
        if (result == -1) { 
          perror("target dup2()"); 
          exit(2);
        } 
        fcntl(targetFD, F_SETFD, FD_CLOEXEC);
       }
      
      
      if(*background!= 0){                                                              //mark the background flag if true
      
        input_argument[*background_index] = NULL;

      }
  
      //printf("\n input_argument[0]is %s\n", input_argument[0]);
      //printf("\n input_argument[1] is %s \n", input_argument[1]);
      //printf("\n input_argument[2] is %s \n", input_argument[2]);
      execvp(input_argument[0], input_argument);
      perror("execvp"); 
      exit(2);
      break;
    
    default:                                                                               //because in test2, we using it as the background process, so we using 
                                                                                            //waitpid(spawnPid, &childStatus,WNOHANG); WNOHANG will no longer wait, so it will run in background
      printf("background pid is %d\n",spawnPid);
      fflush(stdout); 
      spawnPid = waitpid(spawnPid, &childStatus,WNOHANG);
      int background_process_count = 0;
      
      
      while((spawnPid = waitpid(-1, &childStatus, WNOHANG)) > 0){                            // if there detect there are running background, 
      
      if(WIFEXITED(childStatus)){                                                            //check the process is done,
  
      fflush(stdout);
      printf("background pid %d is done: exit value %d\n", spawnPid, WEXITSTATUS(childStatus));
      fflush(stdout);
      
      }
      else{                                                                                          //or the background exit by other signal
      printf("background pid %d is done: terminated by signal %d\n", spawnPid, WTERMSIG(childStatus));
      fflush(stdout);
      
      }
      
      }
      
      
      break;
  }
}

void catchSIGTSTP(int sig){                                                  // a simple catch sig function, this mean to find the ctrl + z, so when using  ctrl + z, it will in the 

  if(foreground_only == 0){                                                        //foreground_only mode, type ctrl + z again, it will exit the mode.
    printf("\nEntering foreground-only mode (& is now ignored)\n");
    fflush(stdout);
    
    foreground_only = 1;
  }
  else if (foreground_only == 1){                                            //if foreground_only == 1, it means it will exting and reset the value of foreground_only
    printf("\nExiting foreground-only mode\n");
    fflush(stdout);
    foreground_only = 0;
  }
  //}
  //else if (background == 0){
    //printf()
  //}

}


int main(){
  char user_input[2048] = "";                  //set up all the variable that will using in the next part
  char * input_argument[512];
  int arugment_index;
  
  char input_file[100] = "";

  char output_file[100] = "";

  int background = 0;
  
  int background_index = 0;
  int input_index = 0;  
  int output_index = 0;
  int clear;
  int childStatus_result;
  int reamber_pid;
  
  //ignore SIGINT_action ctrl c
  
  struct sigaction SIGINT_action = {0};                                          //control signal from ctrl c, from leacture and canvas
  SIGINT_action.sa_handler = SIG_IGN;                                             //SIG_IGN is the defualt catching signal, it will catch ctrl+c 
  sigfillset(&SIGINT_action.sa_mask);
  SIGINT_action.sa_flags = 0;
  sigaction(SIGINT, &SIGINT_action, NULL);
  
      
  //ignore SIGTSTP ctrl z                                                      //control signal from ctrl z, from leacture and canvas
  struct sigaction SIGTSTP_action = {0};
  SIGTSTP_action.sa_handler = catchSIGTSTP;                                     //catchSIGTSTP is the function that i created to catching the ctrl+z singal
  sigfillset(&SIGTSTP_action.sa_mask);
  SIGTSTP_action.sa_flags = 0;
  sigaction(SIGTSTP, &SIGTSTP_action, NULL);
  
  while(1)
  {
    //clean arugment
    //printf("exit value %d\n", childStatus_result);  
    fflush(stdout);
    
    
    printf(": ");
    fflush(stdout);
      
      
    int clear;

    
    for(clear = 0;clear < 512;clear++)                                    //clean everything for var.
    {
      input_argument[clear] = NULL;
      clear++;
    }
    
    memset(user_input, '\0', sizeof(user_input));
    memset(input_file, '\0', sizeof(input_file));
    memset(output_file, '\0', sizeof(output_file));
    input_index = 0;
    output_index = 0;
    arugment_index = 0;
    background = 0;
    background_index = 0;
    
    fgets(user_input, 2048, stdin);
    if(user_input[0] == '\n' || user_input[0] == '#')              //if user enter space or enter, keep running
    {
      // user enter nothing
      continue;
    } 
    
    else if(strcmp(user_input,"exit\n") == 0)                      //checking all condition for build in command
    {
      exit(0);
    }  
    
    else if(strcmp(user_input,"status\n") == 0)                    //print out the exit value as status
    {
      printf("exit value %d\n", childStatus_result);  
      fflush(stdout);
      
    }
    
    else if(strstr(user_input,"$")){                                 //print out $
      pid_t pid = getpid();
      
      printf("%d\n",pid);
      fflush(stdout);
    
    }
  
    else if(strcmp(user_input,"cd\n") == 0){                          //build in fuction cd, return to the pwd of home
        chdir(getenv("HOME"));
    }
      

                                                  // if not build in, go in to the execvp()
    else{
    generate_argument(user_input,input_argument, &arugment_index,input_file,output_file, &background,&input_index,&output_index,&background_index);
    

     
     
      if(foreground_only == 1){                    //if user in the foreground_only mode,
        if(background != 1 ){                      //only processing command that without "&", if they do have "&" in command, ignore
        test(input_argument,&arugment_index,input_file,output_file, &background,&input_index,&output_index,&background_index,&childStatus_result);
        }
      }
        
      else{                          //if not forgound only, check backgound &
      
        if(background == 0){        //if background is 0, means off, so running test the forground mode.
        test(input_argument,&arugment_index,input_file,output_file, &background,&input_index,&output_index,&background_index,&childStatus_result); 
        }
        else{
                                        //if background is 1, means it on, so using test2 to run in the background mode.
        test2(input_argument,&arugment_index,input_file,output_file, &background,&input_index,&output_index,&background_index,&childStatus_result); 
        
        }
      }
      
      
      
    }
  }
  return 0;
}



