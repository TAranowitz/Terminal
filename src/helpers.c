// Your helper functions need to be here.
#include "helpers.h"
#include "icssh.h"
#include "linkedList.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>


void cdBuiltIn(job_info *job)
{
  char s[100];
  //fprintf(stdout, "%s\n", getcwd(s,100));
  //printf("called cd target: %s result:%d\n", job->procs->argv[1], chdir(job->procs->argv[1]) );
  int status = -1;
      
  if(job->procs->argc == 1)
  {
    status = chdir(getenv("HOME"));
  }
  else if(job->procs->argc == 2)
  {
    status = chdir(job->procs->argv[1]);
  }
      
  if(status == 0)
  {
    fprintf(stdout, "%s\n", getcwd(s,100));
  }
  else
  {
    fprintf(stderr, "%s", DIR_ERR);
  }
}

int bgentryComparator(void * l, void * r){
  bgentry_t * left = l;
  bgentry_t * right = r;

  if(left->seconds < right ->seconds)
  {
    return -1;
  }
  else if (left->seconds == right ->seconds)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

void freeBGList(List_t * bgList)
{
  node_t * curr = bgList ->head;

  while(curr != NULL) //go through nodes and free the bgEntry
  {
    bgentry_t * bg = curr ->value;
    if(bg->job!=NULL)
    {
      free_job(bg->job);
    }
    
    free(bg);
    curr = curr ->next;
  }
  deleteList(bgList); //deletes nodes in bglist
  free(bgList); // delete list struct itself
}




void print_bgList(List_t * bgList)
{
  node_t * curr = bgList ->head;

  while(curr != NULL) //go through nodes and free the bgEntry
  {
    bgentry_t * bg = curr ->value;

    if(bg->job!=NULL)
    {
      print_bgentry(bg);
    }
    
    curr = curr ->next;
  }
}

void reapBGList(List_t * bgList)
{
  int exit_status;
  //wait_result = waitpid(pid, &exit_status, WNOHANG);
  node_t * curr = bgList ->head;
  int i = 0;
  while(curr != NULL) //go through nodes and free the bgEntry
  {
    bgentry_t * bg = curr ->value;
    waitpid(bg->pid, &exit_status, WNOHANG);
    //printf("%d\n", exit_status);
    if(WEXITSTATUS(exit_status) == 0 || WEXITSTATUS(exit_status) == 1)
    {
      printf(BG_TERM,bg->pid,bg->job->line);
      curr = curr ->next;
      
      if(bg->job!=NULL)
      {
        free_job(bg->job);
      }
      free(bg);
      removeByIndex(bgList, i);
    }
    else
    {
      i++;
      curr = curr ->next;
    }
  }
}

void killBGList(List_t * bgList)
{
  int exit_status;
  //wait_result = waitpid(pid, &exit_status, WNOHANG);
  node_t * curr = bgList ->head;
  int i = 0;
  //printf("wazdda");
  while(curr != NULL) //go through nodes and free the bgEntry
  {
    bgentry_t * bg = curr ->value;
    printf(BG_TERM,bg->pid,bg->job->line);
    curr = curr ->next;
    

    if(bg->job!=NULL)
    {
      free_job(bg->job);
    }
    free(bg);
    removeByIndex(bgList, i);
    
  }
  deleteList(bgList); //deletes nodes in bglist
  free(bgList); // delete list struct itself
}



//inspired from csapp library
ssize_t safePrintString(char s[])
{
  return write(1, s, mystrLen(s));
}

size_t mystrLen(char s[])
{
    int i = 0;
    while (s[i] != '\0')
        ++i;
    return i;
}


ssize_t safePrintLong(long v)
{
    char x[100];
    int i,c = 0;

    //convert long to string but will be inverted
    do {  
        x[i++] = ((c = (v % 10)) < 10)  ?  c + '0' : c - 10 + 'a';
    } while ((v /= 10) > 0);

    x[i] = '\0';
    int j;

    //reverse the string
    for (i = 0, j = mystrLen(x)-1; i < j; i++, j--) {
        c = x[i];
        x[i] = x[j];
        x[j] = c;
    }
    
    return safePrintString(x);
}

int numPipes(job_info *job)
{
  int count = -1;
  proc_info * proc = job ->procs;
  while(proc != NULL)
  {
    count ++;
    proc = proc -> next_proc;
  }
  return count;
}
