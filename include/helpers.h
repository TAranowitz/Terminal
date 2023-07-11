// A header file for helpers.c
// Declare any additional functions in this file

#include "icssh.h"
#include "linkedList.h"

void cdBuiltIn(job_info *job);
int bgentryComparator(void * l, void * r);
void freeBGList(List_t * bgList);
void print_bgList(List_t * bgList);
void reapBGList(List_t * bgList);
void killBGList(List_t * bgList);
ssize_t safePrintString(char s[]);
ssize_t safePrintLong(long v);
size_t mystrLen(char s[]);
int numPipes(job_info *job);
