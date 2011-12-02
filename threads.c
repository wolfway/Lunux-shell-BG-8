#include "common.h"
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PROMPT       "bg_8>"


extern proc_info_rec_t proc_info_rec;
extern struct bg_plist * head_bg_list;
sigset_t sb;
sigset_t sb_child;
extern struct sigaction sa_kill;
typedef struct{

 pthread_cond_t  cond;
 pthread_mutex_t mutex;
 int b_flag_full;

}sem_cond_t;

/* Variables */
sem_cond_t mutex_cond;
sem_cond_t *p_mutex_cond=&mutex_cond;
char arr_input_buff[MAXLINE_LEN + 1];
char * arr_parsed_command[PARSED_COMMAND_LENGHT];
int i_fg_pid = 0;
extern  sem_t  sf_fg_process;
void v_block_unblock_stdin(int block_flag)
{
  ioctl(0, FIONBIO, &block_flag);

 /* val = fcntl(sock, F_GETFL, 0); */
 /* fcntl(sock, F_SETFL, val|O_NONBLOCK); */
}
/***********************************************/
/* function name : v_block_unblock_stdin()      */
/* return falue  :                             */
/***********************************************/
/***********************************************/
/* function name : v_clear_input_buffer()      */
/* return falue  :                             */
/***********************************************/
void v_too_long_command()
{
int i_in_count = 0;
  printf("!# Prilis dlouhy vstup !!!\n");
  v_block_unblock_stdin(1); /* makes stdin non-blocked */

    while(i_in_count != -1){
       i_in_count=read(0,arr_input_buff,MAXLINE_LEN + 1);
 }

  v_block_unblock_stdin(0);   /* makes stdin blocked */

}
/***********************************************/
/* function name : v_clear_input_buffer()      */

/***********************************************/
void v_clear_input_buffer(){

 if (arr_input_buff != memset(arr_input_buff,'\0',MAXLINE_LEN + 1) )
  {
    printf("memset() error");
    return;
  }

}
/***********************************************/
/* function name : v_shell_exe_read()          */
/* return falue  :                             */
/***********************************************/
void mutex_cond_init(){

 pthread_mutex_init(&p_mutex_cond->mutex, NULL);
 pthread_cond_init(&p_mutex_cond->cond,NULL);
 p_mutex_cond->b_flag_full=FALSE;
}/* end : mutex_cond_init() */
/***********************************************/
/* function name : pv_shell_CLI_thread()       */
/* return falue  :                             */
/***********************************************/
void * pv_shell_CLI_thread()
{
void *pv_ret_value = NULL;
int i_command_len = 0;


   while(1){

        pthread_mutex_lock(&p_mutex_cond->mutex);
 
        while( p_mutex_cond->b_flag_full == TRUE ){  /* Wait for un empty buffer*/
            pthread_cond_wait(&p_mutex_cond->cond,&p_mutex_cond->mutex);
        }

        printf(PROMPT);
        fflush(stdout);

        /* write to the input Buffer */
        i_command_len = read(0,arr_input_buff,MAXLINE_LEN + 1);
        /* check if read() has finished normaly */
        if(i_command_len == -1 ){
          perror("input_thread");
          return(pv_ret_value);
	}/*if*/

        if(i_command_len == 1 ){/* the user just pressed enter --  no sence to bother the other thread */
          pthread_mutex_unlock(&p_mutex_cond->mutex);
          v_clear_input_buffer();
          continue;
        } /*if*/

        if( i_command_len > MAXLINE_LEN){
          v_too_long_command();
          pthread_mutex_unlock(&p_mutex_cond->mutex);
          v_clear_input_buffer(); 
          continue;
        } /* if */
        else{
            p_mutex_cond->b_flag_full = TRUE;
            pthread_cond_broadcast(&p_mutex_cond->cond);
            pthread_mutex_unlock(&p_mutex_cond->mutex);
       }/*else*/
  } /* while */

  return pv_ret_value;
}/* end : pv_shell_CLI_thread()*/
/***********************************************/
/* function name : i_create_new_process_fork() */
/* return falue  :                             */
/***********************************************/
int i_create_new_process_fork(){
int i_pid = 0;
int exec_ret;

   errno = 0;
   i_pid= fork(); /* create new process  */

   if (i_pid < 0 ){
     printf("#Cannot fork !!! \n");
     v_global_shell_destructor();
     exit(1);
   }
   if( i_pid == 0 ){ /* replace the code of the child process */
   
          if (proc_info_rec.stdin_ != NULL){ /* input */
             proc_info_rec.fd_input=open(proc_info_rec.stdin_,O_RDONLY);
             while( close(0)== -1 );
             while(dup2(proc_info_rec.fd_input,0) == -1 ); 
             free(proc_info_rec.stdin_);
             proc_info_rec.stdin_ = NULL;
          }/* input */
    
           if (proc_info_rec.stdout_ != NULL){ /* output */
             proc_info_rec.fd_output=open(proc_info_rec.stdout_,O_CREAT|O_TRUNC|O_WRONLY,S_IRWXU);/* Open/Create the output file */ 
             while( close(1) == -1 );
             while( dup(proc_info_rec.fd_output) == -1 ); 
             free(proc_info_rec.stdout_);
             proc_info_rec.stdout_ = NULL;
           }/* output */

    exec_ret = execvp (*arr_parsed_command,arr_parsed_command);
    perror(" execcvp - i_create_new_process_fork");
    sleep(1);
    exit(ERROR);
 }/* i_pid == 0 */
 return i_pid;
}
/***********************************************/
/* function name : pv_shell_exe_read()         */
/* return falue  :                             */
/***********************************************/
void *pv_shell_exec_thread()
{
int i_pid_child = 0;
int p_is_command = FALSE;
void *pv_ret_value = NULL;

 while(1){

     
     pthread_mutex_lock(&p_mutex_cond->mutex);

     while( p_mutex_cond->b_flag_full == FALSE ){
         pthread_cond_wait(&p_mutex_cond->cond,&p_mutex_cond->mutex);
     }/* while */

    p_is_command = FALSE;

    if( b_parser() == OK){
             
          /* Shell Command exit */
          if( strncmp(arr_parsed_command[0],"exit",4) == 0 ){

            printf("!# Nashledanou !!!\n");
            v_global_shell_destructor();
          			
            exit(0);
          }/* if exit */

          /* Shell Command fg */
          if( strncmp(arr_parsed_command[0],"fg",2) == 0 ){
            printf("!# fg is not implemented\n"); 
            p_is_command = TRUE;
          }/*if fg*/
          /* Shell Command bg */
          if( strncmp(arr_parsed_command[0],"bg",2) == 0 ){
            printf("!# bg is not implemented\n");  
            p_is_command = TRUE;
          }/*if bg */

          /* Shell Command Jobs */
          if( strncmp(arr_parsed_command[0],"jobs",3) == 0 ){

            sigprocmask(SIG_BLOCK, &sb_child, NULL);

            v_print_jobs_list(); /* List of the current jobs */
            i_remove_item_linked_list(); /* Remove all jobs that has status DONE from the list */
            p_is_command = TRUE;
   
            sigprocmask(SIG_UNBLOCK, &sb_child, NULL);
            
          }/*if Jobs*/

          if(!p_is_command){
              /* if & - Background process*/
               if(proc_info_rec.b_bg_process == TRUE){

                    i_pid_child=i_create_new_process_fork();
                    if(i_pid_child != ERROR && errno == 0){
                   

                              if( head_bg_list == NULL){
 
   
                                  sigprocmask(SIG_BLOCK, &sb_child, NULL);

                                  if( i_set_up_linked_list(i_pid_child) != OK ){
                                       printf("set_up_linked_list Fatal Error\n");
                                       exit(1);
                                   }/*if*/ 

                                   sigprocmask(SIG_UNBLOCK, &sb_child, NULL);
                              }/*if head_bg_list*/
                              else{
                                   sigprocmask(SIG_BLOCK, &sb_child, NULL);
                                     
                                  if(i_add_to_end_linked_list(i_pid_child) != OK ){
                                     printf("add_to_end_linked_list Fatal Error\n");
                                     exit(ERROR);
                                  }/*if*/
                                  sigprocmask(SIG_UNBLOCK, &sb_child, NULL);
                              }/*else*/ 
                  }/* if have to add to the list */
              }/*if BackGround Process*/ 
              else{/* Start Normal Process*/
                  /* execute program fg */

                  /* Allow Ctrl + C  */ 
                  sigprocmask(SIG_UNBLOCK, &sb, NULL);                  

                  i_pid_child = i_create_new_process_fork();
                  
                  i_fg_pid=i_pid_child; /* The Process Id - SIGINT -  in case of Ctrl + C */
                  while (sem_wait(&sf_fg_process) == -1 );
                   
                  /* Forbit Ctrl + C */ 
                  sigprocmask(SIG_BLOCK, &sb, NULL);
              }/*else Normal Process*/

          }/*if(!p_is_command)*/ 



    }/*if b_parser*/
    /* The buffer was processed.*/
    p_mutex_cond->b_flag_full = FALSE;
   /* Clear the input buffer*/
    v_clear_input_buffer();  
   /* sends signal to CLI thread */
   pthread_cond_broadcast(&p_mutex_cond->cond);
  /* Unlock the mutex */
   pthread_mutex_unlock(&p_mutex_cond->mutex);

 }/*while*/
 return pv_ret_value;
}/* end : void *pv_shell_exec_thread() */
/***********************************************/
/* function name : b_pthreads_init()           */
/* return falue  : ERROR / OK                  */
/***********************************************/
int  b_pthreads_init(){

int i=0,result=0;
void *pv_ret_value_cli = NULL;
void *pv_ret_value_exec = NULL;
pthread_attr_t attr[2];
pthread_t pt[2];


 /*  init thread's attributs */
  for(i=0;i<2;i++){
   if( ( result = pthread_attr_init(&attr[i])) != 0 ){
   printf(" pthread_attr_init() err %d \n",result);
   return ERROR;
   }
  } /* for */

  /*  set thread's attributs */
  for(i=0;i<2;i++){
   if( (result = pthread_attr_setdetachstate(&attr[i],PTHREAD_CREATE_JOINABLE)) != 0 ){
   printf(" pthread_attr_setdetachstate () err %d \n",result);
   return ERROR;
   }
  } /* for */

  /*  create and start  Command Line Interface thread */

  result = pthread_create(pt,attr,pv_shell_CLI_thread,NULL);
  if(result){
    printf(" pthread_create() err %d\n",result);
    return ERROR;
  }
 /*  create and start Shell_Exec  thread */
   result = pthread_create(&pt[1],&attr[1],pv_shell_exec_thread,NULL);
  if(result){
    printf(" pthread_create() err %d\n",result);
    return ERROR;
  }
   /* wait to finish first thread */
  if( (result = pthread_join(pt[0],&pv_ret_value_cli)) != 0 ){
    printf("pthread_join() err %d\n",result);
    return ERROR;
  }

   /* wait to finish second thread */
  if( (result = pthread_join(pt[1],&pv_ret_value_exec)) != 0 ){
    printf("pthread_join() err %d\n",result);
    return ERROR;
  }

  return OK;

} /* end : int  b_pthreads_init() */

