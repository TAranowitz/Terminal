#include "icssh.h"
#include <readline/readline.h>
#include <unistd.h>
#include "helpers.h"


volatile int bgFlag = 0;

void child_handler(int sig)
{
  bgFlag = 1;
}
void sigusr2_handler(int sig)
{
  safePrintString("Hi User! I am process ");
  safePrintLong((long)getpid());
  safePrintString("\n");
}
int main(int argc, char* argv[]) {
  int exec_result;
  int exit_status;
  pid_t pid;
  pid_t wait_result;
  char* line;
#ifdef GS
  rl_outstream = fopen("/dev/null", "w");
#endif

  // Setup segmentation fault handler
  if (signal(SIGSEGV, sigsegv_handler) == SIG_ERR) {
    perror("Failed to set signal handler");
    exit(EXIT_FAILURE);
  }
  //setup sigchild handler
  if (signal(SIGCHLD, child_handler) == SIG_ERR) {
    perror("Failed to set signal handler");
    exit(EXIT_FAILURE);
  }
  //setup sigsur2 handler
  if (signal(SIGUSR2, sigusr2_handler) == SIG_ERR) {
    perror("Failed to set signal handler");
    exit(EXIT_FAILURE);
  }

  //create bg  linked list
  List_t * bgList = malloc(sizeof(List_t));
  bgList -> head = NULL;
  bgList -> length = 0;
  bgList -> comparator = &bgentryComparator;
   
  
  // print the prompt & wait for the user to enter commands string
  while ((line = readline(SHELL_PROMPT)) != NULL) {

    if(bgFlag == 1){
      reapBGList(bgList);
      bgFlag = 0;
    }
    time_t sec = time(NULL);
    // MAGIC HAPPENS! Command string is parsed into a job struct
    // Will print out error message if command string is invalid
    job_info* job = validate_input(line);
    if (job == NULL) { // Command was empty string or invalid
      free(line);
      continue;
    }
    

    //Prints out the job linked list struture for debugging
#ifdef DEBUG   // If DEBUG flag removed in makefile, this will not longer print
    debug_print_job(job);
#endif

    // example built-in: exit
    if (strcmp(job->procs->cmd, "exit") == 0) {
      // Terminating the shell
      free(line);
      free_job(job);
      //freeBGList(bgList);
      killBGList(bgList);
      validate_input(NULL);   // calling validate_input with NULL will free the memory it has allocated
      return 0;
    }
    //built-in cd
    else if (strcmp(job->procs->cmd, "cd") == 0) {
    
      cdBuiltIn(job);
    }
    else if (strcmp(job->procs->cmd, "estatus") == 0) //built-in exit status
    {
      
      fprintf(stdout, "%d\n", WEXITSTATUS(exit_status));
    }
    else if(strcmp(job->procs->cmd, "bglist") == 0)//built in bgList
    {
      print_bgList(bgList);
    }
    else if(numPipes(job) == 0)//if not built in
    {
      // example of good error handling!
      if ((pid = fork()) < 0) {
        perror("fork error");
        exit(EXIT_FAILURE);
      }
      if (pid == 0) {  //If zero, then it's the child process
        
        //get the first command in the job list
        proc_info* proc = job->procs;
        if(job->out_file != NULL)
          {
            int newOut  = open(job->out_file, O_WRONLY | O_CREAT,0644);
            if(newOut<0)
              {
                fprintf(stderr, RD_ERR);;
                exit(1);
              }
            else
              {
                if(dup2(newOut, 1) == -1)
                  {
                    fprintf(stderr, RD_ERR);
                  }
                close(newOut);
              }      
          }
        if(job->in_file != NULL)
          {   
            int newIn  = open(job->in_file, O_RDONLY);    
            if(newIn<0)
              {
                fprintf(stderr, RD_ERR);
                exit(1);
              }       
            else
              {
                if(dup2(newIn, 0) == -1)
                  {
                    fprintf(stderr, RD_ERR);
                  }
                close(newIn);
              }
          
          
          }
        if(job->procs->err_file != NULL)
          {
          
            int newErr  = open(job->procs->err_file, O_WRONLY | O_CREAT,0644);
            if(newErr<0)
              {
                fprintf(stderr, RD_ERR);
                exit(1);
              }
            else
              {
                if(dup2(newErr, 2) == -1)
                  {
                    fprintf(stderr, RD_ERR);
                  }
                close(newErr);
              }
          }
        

        
        exec_result = execvp(proc->cmd, proc->argv);
        if (exec_result < 0) {  //Error checking
          printf(EXEC_ERR, proc->cmd);
				
          // Cleaning up to make Valgrind happy 
          // (not necessary because child will exit. Resources will be reaped by parent)
          free_job(job);  
          free(line);
          freeBGList(bgList);
          
          validate_input(NULL);  // calling validate_input with NULL will free the memory it has allocated

          exit(EXIT_FAILURE);
        }
        
      }
      else { //if parent

        if(! (job->bg))//if job was not a bg process
        {
          // As the parent, wait for the foreground job to finish
          wait_result = waitpid(pid, &exit_status, 0);
          if (wait_result < 0) {
            printf(WAIT_ERR);
            exit(EXIT_FAILURE);
          }
        }
        else//if job was a bg process;
        {
          bgentry_t * newBG = malloc(sizeof(bgentry_t));
          newBG -> job = job;
          newBG -> pid = pid;
          newBG -> seconds = sec;
          insertInOrder(bgList, newBG);

          //printf("bgList length: %d\n", bgList->length);
          //freeBGList(bgList);
          
        }  
      }
    }
    else if(numPipes(job) == 1)
    {
      if ((pid = fork()) < 0) {
        perror("fork error");
        exit(EXIT_FAILURE);
      }
      
      if (pid == 0)
      {  //THIS WILL BE THE WRITE CHILD
        int p[2] = {0,0};
        if(pipe(p) == -1)
        {
          fprintf(stderr,"pipe() error\n");
        }


        //FORK FOR SECOND PROCESS
        int pid2;
        if ((pid2 = fork()) < 0) {
          perror("fork error");
          exit(EXIT_FAILURE);
        }
        if (pid == 0) // if SECOND PROCESS THE READER
        {
          proc_info* proc = job->procs->next_proc;
          //THIS WILL BE THE WRITE CHILD
          close(p[1]);//close write end
          //dup p[0] to stdin
          if(dup2(p[0], 0) == -1)
          {
            fprintf(stderr, RD_ERR);
          }
          close(p[0]);
          
          exec_result = execvp(proc->cmd, proc->argv);
          if (exec_result < 0) {  //Error checking
            printf(EXEC_ERR, proc->cmd);
          
            // Cleaning up to make Valgrind happy 
            // (not necessary because child will exit. Resources will be reaped by parent)
            free_job(job);  
            free(line);
            freeBGList(bgList);
            
            validate_input(NULL);  // calling validate_input with NULL will free the memory it has allocated

            exit(EXIT_FAILURE);
          }
        
        }
        else // if first process WRITER
        {
          proc_info* proc = job->procs;
          close(p[0]);//clos read end
          //dup p[1] to stdout
          if(dup2(p[1], 1) == -1)
          {
            fprintf(stderr, RD_ERR);
          }
          close(p[1]);

          exec_result = execvp(proc->cmd, proc->argv);
          if (exec_result < 0) {  //Error checking
            printf(EXEC_ERR, proc->cmd);
          
            // Cleaning up to make Valgrind happy 
            // (not necessary because child will exit. Resources will be reaped by parent)
            free_job(job);  
            free(line);
            freeBGList(bgList);
            
            validate_input(NULL);  // calling validate_input with NULL will free the memory it has allocated

            exit(EXIT_FAILURE);
          }

        }


        //get the first command in the job list
      }
      else { //if parent of parent, shell process

        if(! (job->bg))//if job was not a bg process
        {
          // As the parent, wait for the foreground job to finish
          wait_result = waitpid(pid, &exit_status, 0);
          if (wait_result < 0) {
            printf(WAIT_ERR);
            exit(EXIT_FAILURE);
          }
        }
        else//if job was a bg process;
        {
          bgentry_t * newBG = malloc(sizeof(bgentry_t));
          newBG -> job = job;
          newBG -> pid = pid;
          newBG -> seconds = sec;
          insertInOrder(bgList, newBG);

          //printf("bgList length: %d\n", bgList->length);
          //freeBGList(bgList);
          
        }  
      }
      
    }//this is end of if numPipe==1 block
    else if(numPipes(job) == 2)
    {
      int p[2] = {0,0};
      if(pipe(p) == -1)
      {
        fprintf(stderr,"pipe() error\n");
      }

      /*fork twice, first for write program and second time for read program
      each time in between make sure to wait or add to bg process
      */
      if ((pid = fork()) < 0) {
        perror("fork error");
        exit(EXIT_FAILURE);
      }
      
      if (pid == 0) {  //THIS WILL BE THE WRITE CHILD
        close(p[0]);
        //get the first command in the job list
        proc_info* proc = job->procs;
        
        //dup p[1] to stdout
        if(dup2(p[1], 1) == -1)
        {
          fprintf(stderr, RD_ERR);
        }
        close(p[1]);
        
        exec_result = execvp(proc->cmd, proc->argv);
        if (exec_result < 0) {  //Error checking
          printf(EXEC_ERR, proc->cmd);
				
          // Cleaning up to make Valgrind happy 
          // (not necessary because child will exit. Resources will be reaped by parent)
          free_job(job);  
          free(line);
          freeBGList(bgList);
          
          validate_input(NULL);  // calling validate_input with NULL will free the memory it has allocated

          exit(EXIT_FAILURE);
        }
      }
      else { //if parent
        if(! (job->bg))//if job was not a bg process
          {
            // As the parent, wait for the foreground job to finish
            wait_result = waitpid(pid, &exit_status, 0);
            if (wait_result < 0) {
              printf(WAIT_ERR);
              exit(EXIT_FAILURE);
            }
          }
        else//if job was a bg process;
          {
            bgentry_t * newBG = malloc(sizeof(bgentry_t));
            newBG -> job = job;
            newBG -> pid = pid;
            newBG -> seconds = sec;
            insertInOrder(bgList, newBG);

            //printf("bgList length: %d\n", bgList->length);
            //freeBGList(bgList);
          
          }
      }
      


      /*
      *NEXT FORK A SECOND TIME BUT THIS TIME FOR THE READER CHILD
      * READER CHILD
      */



      int p2[2] = {0,0};
      if(pipe(p2) == -1)
      {
        fprintf(stderr,"pipe() error\n");
      }
      close(p[1]);
      if ((pid = fork()) < 0) {
        perror("fork error");
        exit(EXIT_FAILURE);
      }
      if (pid == 0) {  //THIS WILL BE THE WRITE CHILD
        //printf("in child of second process");
        //get the first command in the job list
        proc_info* proc = job->procs->next_proc;
        //close(p[1]);
        //dup p[0] to stdin
        if(dup2(p[0], 0) == -1)
        {
          fprintf(stderr, RD_ERR);
        }
        close(p[0]);
        

        close(p2[0]);
        
        //dup p[1] to stdout
        if(dup2(p2[1], 1) == -1)
        {
          fprintf(stderr, RD_ERR);
        }
        close(p2[1]);


        exec_result = execvp(proc->cmd, proc->argv);
        if (exec_result < 0) {  //Error checking
          printf(EXEC_ERR, proc->cmd);
				
          // Cleaning up to make Valgrind happy 
          // (not necessary because child will exit. Resources will be reaped by parent)
          free_job(job);  
          free(line);
          freeBGList(bgList);
          
          validate_input(NULL);  // calling validate_input with NULL will free the memory it has allocated

          exit(EXIT_FAILURE);
        }
      }
      else { //if parent of second process
        if(! (job->bg))//if job was not a bg process
          {
            // As the parent, wait for the foreground job to finish
            wait_result = waitpid(pid, &exit_status, 0);
            if (wait_result < 0) {
              printf(WAIT_ERR);
              exit(EXIT_FAILURE);
            }
          }
        
      }
      //close(p[1]);
      close(p[0]);


      /*
      *NEXT MAKE THIRD CHILD FOR THIRD PROCESS
      */
      close(p2[1]);

      if ((pid = fork()) < 0) {
        perror("fork error");
        exit(EXIT_FAILURE);
      }
      if (pid == 0) {  //THIS WILL BE THE WRITE CHILD
        //printf("in child of second process");
        //get the first command in the job list
        proc_info* proc = job->procs->next_proc->next_proc;
        //close(p[1]);
        //dup p2[0] to stdin
        if(dup2(p2[0], 0) == -1)
        {
          fprintf(stderr, RD_ERR);
        }
        close(p2[0]);
        
        exec_result = execvp(proc->cmd, proc->argv);
        if (exec_result < 0) {  //Error checking
          printf(EXEC_ERR, proc->cmd);
				
          // Cleaning up to make Valgrind happy 
          // (not necessary because child will exit. Resources will be reaped by parent)
          free_job(job);  
          free(line);
          freeBGList(bgList);
          
          validate_input(NULL);  // calling validate_input with NULL will free the memory it has allocated

          exit(EXIT_FAILURE);
        }
      }
      else
      { //if parent of second process
        if(! (job->bg))//if job was not a bg process
        {
          // As the parent, wait for the foreground job to finish
          wait_result = waitpid(pid, &exit_status, 0);
          if (wait_result < 0) {
            printf(WAIT_ERR);
            exit(EXIT_FAILURE);
          }
        }
      }


      close(p2[0]);


    }//this is end of numPipe == 2 block

    
    if(! (job->bg))//if job was not a bg process
    {
      free_job(job);  // if a foreground job, we no longer need the data
      
    }
    free(line);
  }

  // calling validate_input with NULL will free the memory it has allocated
  validate_input(NULL);

#ifndef GS
  fclose(rl_outstream);
#endif
  return 0;
}
