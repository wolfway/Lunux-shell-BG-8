#define _XOPEN_SOURCE 500
#define _POSIX_VERSION 199506L

#include "common.h"
#include <stdio.h>
#include <semaphore.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

sem_t sf_fg_process;

extern proc_info_rec_t proc_info_rec;
/* Declarations */
void v_global_shell_init();
void v_global_shell_destructor();
int main(int argc, char **argv)
{
printf("**********************************************************\n");
printf("*                  Project POS - 3                       *\n");
printf("*      Writen by Dimitar Yordanov - xjorda01             *\n");
printf("*              How to Use this Shell                     *\n");
printf("*  All should be separate strings !!!                    *\n");
printf("*  > redirects output                                    *\n");
printf("*  < redirects input                                     *\n");
printf("*  & starts a background process                         *\n");
printf("*  | pipe is not implemented yet                         *\n");
printf("*  Exanple : cat  /etc/resolv.conf > resolv.save &       *\n");
printf("*  Shell Commands                                        *\n");
printf("*   exit - exit the shell                                *\n");
printf("*   jobs - show the list of Bg jobs                      *\n");
printf("*   fg   - not implemented yet                           *\n");
printf("*   bg   - not implemented yet                           *\n");
printf("**********************************************************\n");
  /* Set up init values to the Shell Env*/
  v_global_shell_init();
 /* Creates two threads */
 if ( b_pthreads_init() != OK ){
    /* Creating of a thread failed */
     return ERROR;
    }
 
  /* Free all allocated structures */  
  v_global_shell_destructor();

 return OK;  
}
/***********************************************/
/* function name : v_global_shell_inti()       */
/* return falue  :                             */
/***********************************************/
void v_global_shell_init(){

  /* Init the semaphore that synch the end of fg process */
  if( sem_init(&sf_fg_process,0,0) == -1 ){
   perror(" Shell Init Semaphor ");
  }

 /* Initialize the mutex that synchronize the  threads CLI and Exec */
  mutex_cond_init();
 /* Set all Pointer in arr_parsed_init to NULL */
  v_arr_parsed_init();
 /* Init SIGCHLD signal hook  */
 init_SIGCHLD_hook();

}
/***********************************************/
/* function name : v_global_shell_destructor() */
/* return falue  :                             */
/***********************************************/
void v_global_shell_destructor(){

  /*free the memory from the last command*/
  v_arr_parsed_destroy();
 /* Init the semaphore that synch the end of fg process */
  if( sem_destroy(&sf_fg_process) == -1 ){
   perror(" Shell Init Semaphor ");
  }
  /* Detroy the jobs list*/
  i_destroy_linked_list(); 
 
  /* proc_info_rec - release the stdin stdout redirect strings */
  
  if (proc_info_rec.stdin_ != NULL) /* input */
       free(proc_info_rec.stdin_);

  if (proc_info_rec.stdin_ != NULL) /* input */
       free(proc_info_rec.stdin_);
}
