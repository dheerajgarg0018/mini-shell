#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX 1024
#define MAXCOM 100
#define IN 0
#define OUT 1
#define DELIM " \t\n"

#define ANSI_COLOR_BLUE     "\x1b[1;34m"
#define ANSI_COLOR_RESET   "\x1b[0m"
	
// function to print working directory
void printCurrDir()
{
    char dir[1024];
    getcwd(dir, sizeof(dir)); 
    printf(ANSI_COLOR_BLUE "%s " ANSI_COLOR_RESET, dir);
}

// function to take input from user
char* takeInput()
{
    char* str;
    int m=MAX;
    int index = 0;

    str = (char*) malloc(sizeof(char) * m);
    int c;

    if (!str) {
    printf("Input allocation error\n");
    exit(EXIT_FAILURE);
    }

    while (1) {
    c = getchar();

    if (c == '\n') {
      str[index] = '\0';
      return str;
    }
    else {
      str[index] = c;
    }
    index++;

    // If we have exceeded the buffer, reallocate.
      if (index >= m) {
         m += MAX;
         str = (char*) realloc(str, m);

         if (!str) {
            printf("Input reallocation error\n");
            exit(EXIT_FAILURE);
         }
      }
    }

    return(str);
}

// function for help
void Help(){

	printf("\n--------------------Help---------------------\n\n");
	printf("Not all the internal commands are supported.\n");
	printf("Supported internal commands: cd, pwd, echo, history, exit\n");
	printf("Commands can be piped together(only 1 pipe is allowed).\n");
	printf("Ex. ls -a | wc -c is allowed\n");
	printf("Input redirection from file is supported: ex. wc -c < fileInput\n");
	printf("Output redirection to file is supported: ex. ls > fileOutput\n");
	printf("Output redirection to file with append is supported: ex. ls >> fileOutput\n");
	printf("Both Input redirection from file and Output redirection to file in single command is supported.\n");
	printf("Ex. wc -c < fileInput > fileOutput  or  wc -c < fileInput >> fileOutput\n\n");
	
	return;
}

// function to add input to history
void AddHistory(char* input)
{
    FILE* f;
    f = fopen("/tmp/history.txt", "a");

    int i;
    for(i=0; input[i]!='\0'; i++)
    {
        fputc(input[i], f);
    }
    fprintf(f, "\n");

    fclose(f);
}

// function to print history
void printHistory()
{
    FILE* f1;
    f1 = fopen("/tmp/history.txt", "r");

    int c = fgetc(f1);
    while (c != EOF)
    {
        printf("%c", c);
        c = fgetc(f1);
    }

    fclose(f1);
}

// function for parsing command words
void parseInput(char* str, char** parsed)
{
    int i;

    for (i = 0; i < MAXCOM; i++)
    {
        parsed[i] = strsep(&str, DELIM);

        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
}

// function for finding pipe
int searchPipe(char* str, char** piped)
{
    int i;
    for (i = 0; i < 2; i++)
    {
        piped[i] = strsep(&str, "|");

        if (piped[i] == NULL)
            break;
    }

    if (piped[1] == NULL)
        return 0;
    else {
        return 1;
    }
}

// function for finding redirection operation
int searchRedirection(char* str, char** redirect, char** ioredirect)
{
    char* s1= strdup(str);
    
    char* s2= strdup(str);
    
    int i, t=0;
    for (i = 0; i < 2; i++)
    {
        redirect[i] = strsep(&str, "<");

        if (redirect[i] == NULL)
            break;
    }

    if(redirect[1] == NULL)
    {
        redirect[0]=NULL;
        char* sep = strstr(s1, ">>");
        if(sep != NULL){
             size_t l1;
             l1= sep-s1;
             
             char* p = sep + 2;
             redirect[0] = strndup(s1, l1);
             redirect[1] = strdup(p);
             
             t=3;
        }
        else{
            
            for (i = 0; i < 2; i++)
            {
                redirect[i] = strsep(&s2, ">");

                if (redirect[i] == NULL)
                    break;
            }

            if(redirect[1] == NULL){
                t = 0;
            }
            else{
                t = 4;
            }
        
        }
        
    }
    else{
    
        char* sample = strdup(redirect[1]);
        char* sample1 = strdup(redirect[1]);
        
        char* sep1 = strstr(sample, ">>");
        if(sep1 != NULL){
             size_t len;
             len= sep1-sample;
             
             char* p1 = sep1 + 2;
             ioredirect[0] = strndup(sample, len);
             ioredirect[1] = strdup(p1);
             
             t=6;
        }
        else{
            for (i = 0; i < 2; i++)
            {
                  ioredirect[i] = strsep(&sample1, ">");

                  if (ioredirect[i] == NULL)
                       break;
            }
        
            if(ioredirect[1]==NULL) t = 2;
            else t=5;
        }
    }
    
    return t;
}

// Function to execute built-in commands
int internalcmd(char** parsed)
{

    //Checking for environment variables
    if(strcmp(parsed[0], "setenv")==0)
    {
          if(parsed[2] != NULL){
                   
                   if(strcmp(parsed[1], "HOME") != 0)
                           setenv(parsed[1], parsed[3], 1);
                   else
                       setenv(parsed[1], parsed[3], 0);
          } 
          else{
              char* env[2];
              int i;
              char* set = strdup(parsed[1]);
              
              for (i = 0; i < 2; i++)
              {
                    env[i] = strsep(&set, "=");
              }
              
              if(strcmp(env[0], "HOME") != 0)
                           setenv(env[0], env[1], 1);
                   else
                       setenv(env[0], env[1], 0);
          }
          
          return 1;
    }
    
    if(strcmp(parsed[0], "printenv")==0)
    {
          if(getenv(parsed[1]) != NULL)
          {
               printf("%s\n", getenv(parsed[1]));
               
               return 1;
          }
    }
    
    
    //Checking for echo Command
    if(strcmp(parsed[0], "echo")==0)
    {
          if(parsed[1] == NULL){
                  printf("\n");
                  return 1;
          }
          
          if(parsed[1][0]=='$')
          {
               char* ev = parsed[1]+1;
               if(getenv(ev) != NULL){
                         
                      printf("%s\n", getenv(ev));
                      return 1;
               }
               else{
                   printf("\n");
                   return 1;
               }
          }
          
    }


    //Checking for cd Command
    if(strcmp(parsed[0], "cd") == 0)
    {

        //Checking if path is given
        if(parsed[1] == NULL){
            chdir(getenv("HOME"));
            
        }
        else {
            int ch= chdir(parsed[1]);
            if(ch<0) printf("No such file or directory\n");
        }

        return 1;
    }
    
    
    //Checking for help 
    if(strcmp(parsed[0], "help") == 0)
    {
        Help();
        
        return 1;
    }
    
    
    //Checking for history Command
    if(strcmp(parsed[0], "history") == 0)
    {
        printHistory();

        return 2;
    }


    //Checking for exit Command
    if(strcmp(parsed[0], "exit") == 0 || strcmp(parsed[0], "quit") == 0 || strcmp(parsed[0], "x") == 0)
    {
        remove("/tmp/history.txt");
        exit(0);

        return 3;
    }


    return 0;
}

// Function to execute system command
void externalcmd(char** parsed)
{
    // Forking a child
    pid_t pid = fork();

    if (pid == -1)
    {
        printf("Failed forking child.\n");
        return;
    }
    else if (pid == 0)
    {
        if (execvp(parsed[0], parsed) < 0) {
            fprintf(stderr, "%s: Command not found\n", parsed[0]);
            exit(EXIT_FAILURE);
        }
        
        exit(EXIT_SUCCESS);
    }
    else {
        wait(NULL);
        return;
    }
}

// Function to execute piped system commands
void Pipedcmd(char** parsed, char** parsedpipe)
{
    // 0 is read end, 1 is write end
    int fd[2];
    
    
    if(pipe(fd)<0){
         printf("pipe creating was not successful\n");
	 return;
    }
		
    if(fork()==0)
    {//child1
			
	dup2(fd[1],1);
	close(fd[0]);
	close(fd[1]);
			
	int t= internalcmd(parsed);
        if(t==0){		
	
	      if (execvp(parsed[0], parsed) < 0)
              {
                   fprintf(stderr, "%s: Command not found\n", parsed[0]);
                   exit(EXIT_FAILURE);
              }
        
        }
	
	exit(EXIT_SUCCESS);
    }
		
		
     if(fork()==0)
     {//child2
			
	   dup2(fd[0],0);
	   close(fd[1]);
	   close(fd[0]);
			
			
	   int t= internalcmd(parsedpipe);
           if(t==0){
	   
	         if (execvp(parsedpipe[0], parsedpipe) < 0)
                 {
                      fprintf(stderr, "%s: Command not found\n", parsedpipe[0]);
                      exit(EXIT_FAILURE);
                 }
           
           }
           
	   exit(EXIT_SUCCESS);
     }
		
		
     //parent
     close(fd[0]);
     close(fd[1]);
		
     wait(NULL);
     wait(NULL);
    
}

// Function to execute redirection command
void Redirectioncmd(char** parsed, char** redirectfile, int k)
{
     int saved_stdout = dup(STDOUT_FILENO);
     int saved_stdin = dup(STDIN_FILENO);
     
     int fd, fd1, fd2;
     int c=0;
     
     if(fork()==0){		
		
		if(k==2)
		{
                     fd = open(redirectfile[0], O_RDONLY, 0);
		      
		      if(fd<0){
			    fprintf(stderr, "cannot open file\n");
			    exit(EXIT_FAILURE);
		      }
		      
		      dup2(fd, 0);
		
		}
		
		if(k>2 && k<5)
		{
		      if(k==3)
		         fd = open(redirectfile[0], O_WRONLY | O_CREAT | O_APPEND, 0644);
		      else if(k==4)
		         fd = open(redirectfile[0], O_WRONLY | O_CREAT, 0644);
		      
		      if(fd<0){
			fprintf(stderr, "cannot open file\n");
			exit(EXIT_FAILURE);
		      }
		      
		      dup2(fd, 1);
		
		}
		
		if(k>4)
		{
		      fd1 = open(redirectfile[0], O_RDONLY, 0);
		      if(k==5) fd2 = open(redirectfile[1], O_WRONLY | O_CREAT, 0644);
		      if(k==6) fd2 = open(redirectfile[1], O_WRONLY | O_CREAT | O_APPEND, 0644);
		             
		      if(fd1<0){
			      fprintf(stderr, "cannot open input file\n");
			      exit(EXIT_FAILURE);
		      }
		             
		      if(fd2<0){
			      fprintf(stderr, "cannot open output file\n");
			      exit(EXIT_FAILURE);
		      }
		             
		      dup2(fd1, 0);
		      dup2(fd2, 1);
		             
		      c=1;
		
		}
		
	        int t= internalcmd(parsed);
               if(t==0){
               
                   if (execvp(parsed[0], parsed) < 0) 
                   {     
                         fprintf(stderr, "%s: Command not found\n", parsed[0]);
                         exit(EXIT_FAILURE);
                   }
               
               }
               
               if(c==0) close(fd);
               else{
                   close(fd1);
                   close(fd2);
               }
               
               dup2(saved_stdin, 0);
               close(saved_stdin);
        
               dup2(saved_stdout, 1);
               close(saved_stdout);
               
               exit(EXIT_SUCCESS);
                 
	}
	wait(NULL);
	return;
}

// function to check command type
int checkInput(char* str, char** parsed, char** parsedpipe, char** redirectfile)
{
    char* piped[2];
    char* redirect[2];
    char* ioredirect[2];
    int k = 0;

    k = searchPipe(str, piped);

    if (k>0)
    {
        parseInput(piped[0], parsed);
        parseInput(piped[1], parsedpipe);
        
        if(parsed[0]==NULL) printf("Give command before pipe\n");
        else if(parsedpipe[0]==NULL) printf("Give command after pipe\n");
        else 
        Pipedcmd(parsed, parsedpipe);
        
        return (1+k);

    } else {
        k = searchRedirection(str, redirect, ioredirect);
        if(k>0)
        {
            parseInput(redirect[0], parsed);
            if(parsed[0]==NULL) printf("Give command before redirecting\n");
            else{
               if(k==5 || k==6){
                     char* f = (char*) malloc(MAX* sizeof(char));
                     f = strdup(ioredirect[0]);
                     strcat(f, ioredirect[1]);
                
                     parseInput(f, redirectfile);
                     Redirectioncmd(parsed, redirectfile, k);
                
                     free(f);
               }
               else{
                   parseInput(redirect[1], redirectfile);
                   Redirectioncmd(parsed, redirectfile, k);
               }
            }
            
            return (1+k);
        }
        else parseInput(str, parsed);
    }

    int t = internalcmd(parsed);
    
    if(t==3) return -1;
    if (t>0)
        return 0;
    else
        return (1+k);
}

int main()
{
    char* parsed[MAXCOM];
    char* parsedpipe[MAXCOM];
    char* redirectfile[MAXCOM];

    while(1)
    {
        int f=0;

        printCurrDir();
        char* str=takeInput();
        
        if(strcmp(str, "")==0)
        {
             free(str);
             continue;
        }

        char* p= (char*) malloc(MAX*sizeof(char));
        strcpy(p, str);
        
        AddHistory(p);
        
        f = checkInput(str, parsed, parsedpipe, redirectfile);

        if(f==-1) break;
        
        if(f==1)
        {
            externalcmd(parsed);
        }

        free(str);
        free(p);
    }
    
    return 0;
}

