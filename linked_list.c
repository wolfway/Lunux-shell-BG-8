#include "common.h"
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

#define RUNNING                 0
#define DONE                    1
#define STOPPED                 2 /* for future use */

struct bg_plist{
 int pid;
 int number;
 int status; /* 0 - Running    1 - Done  2 - Stopped */
 char * p_command;
 struct bg_plist *prev;
 struct bg_plist *next;
};

struct sigaction sa_kill;
struct bg_plist * head_bg_list = NULL;
struct bg_plist * tail_bg_list = NULL;
int bg_list_counter = 0;
extern char * arr_parsed_command[PARSED_COMMAND_LENGHT];
extern sem_t  sf_fg_process;
extern int i_fg_pid;
extern sigset_t sb;
extern sigset_t sb_child;
/**********************************************/
/* function name : i_remove_item_linked_list() */
/* return falue  :                             */
/***********************************************/
/* This function removes  all items that has status DONE */
int  i_remove_item_linked_list(){

struct bg_plist * bg_index = NULL;
struct bg_plist * temp = NULL;
int i = 0;


 bg_index = head_bg_list;

    while(bg_index != NULL){

    if ( bg_index->status == DONE ){
  
        /* if fist and last*/
        if( bg_index->next == bg_index->prev ){
           free(bg_index);
           head_bg_list = NULL;
           bg_list_counter--;
           return OK;
        }/*if */
        
        /*if first*/
        if( bg_index->prev == NULL ){
            head_bg_list=bg_index->next;
            free(bg_index);
            head_bg_list->prev=NULL;
            bg_index=head_bg_list;
            bg_list_counter--;
            continue;
        }/*if*/

        /* if last */
        if(bg_index->next == NULL){
           temp=bg_index->prev;
           temp->next=NULL;
           free(bg_index);
           bg_list_counter--;
           return OK;
        }
        /* if middle*/
         else{
            temp = bg_index->prev;
            temp->next=bg_index->next;
            free(bg_index->prev);  
            bg_index=temp;
            bg_list_counter--;
        }/* else */

    } /*if  DONE */
    else{
      bg_index=bg_index->next;
    } 
  
  }/*While*/
 return OK;
}
/***********************************************/
/* function name : i_bg_list_in_check()        */
/* return falue  :                             */
/***********************************************/
/* This function will check whether the process with ID is running in background mode */
int i_bg_list_in_check(int pid){
struct bg_plist * bg_index = NULL;

  bg_index = head_bg_list;

  while(bg_index != NULL ){

     if(bg_index->pid == pid){
       bg_index->status = DONE; /* Set the status of the process as DONE */
       return OK;
     }

      bg_index=bg_index->next;
  }

return ERROR;
}

/***********************************************/
/* function name : i_destroy_linked_list()     */
/* return falue  :                             */
/***********************************************/
/* This function free the memory allocated  for a linked list*/
int  i_destroy_linked_list(){

struct bg_plist * new_bg = NULL;

  new_bg=head_bg_list;

  while (new_bg){
   new_bg=head_bg_list->next;
   free(head_bg_list->p_command);
   free(head_bg_list);
   head_bg_list=new_bg;
  }
return OK;
}
/***********************************************/
/* function name : i_print_jobs_list()  */
/* return falue  :                             */
/***********************************************/
/* This function list the current jobs  */
void v_print_jobs_list(){
char *status[]={"RUNNING","DONE","STOPPED"};
struct bg_plist * new_bg = NULL;

   new_bg = head_bg_list;

   while ( new_bg != NULL){
    printf("[%d]   pid =  %d   status: %s  %s \n",new_bg->number,new_bg->pid,status[new_bg->status],new_bg->p_command);
    new_bg=new_bg->next;
  }/*while*/
}

/***********************************************/
/* function name : i_add_to_end_linked_list()  */
/* return falue  :                             */
/***********************************************/
/* This function adds an  new item to the end of the list - new bg process has been started*/
int  i_add_to_end_linked_list(int proc_id)
{
struct bg_plist * new_bg = NULL;

  tail_bg_list->next = malloc(sizeof(struct bg_plist));

  if (tail_bg_list->next == NULL){
   return ERROR;
  }

  new_bg = tail_bg_list->next;
  new_bg->prev = tail_bg_list;
  tail_bg_list = new_bg;
  new_bg->next = NULL;
  bg_list_counter++;
  new_bg->number = bg_list_counter;
  new_bg->pid = proc_id;

 /* Allocate memory for the command that started the process*/
  new_bg->p_command=malloc(strlen(arr_parsed_command[0]) + 1);

  if (new_bg->p_command == NULL){
   return ERROR;
  }
  /* copy the command from the input buffer */
  strncpy(new_bg->p_command,arr_parsed_command[0],strlen(arr_parsed_command[0]));
  new_bg->p_command[strlen(arr_parsed_command[0])] = '\0'; 
 return OK;
}
/***********************************************/
/* function name : i_set_up_linked_list()      */
/* return falue  :                             */
/***********************************************/
/* This function sets up a linked list for the bg processes*/
int  i_set_up_linked_list(int proc_id)
{

    head_bg_list=malloc(sizeof(struct bg_plist));

    if( head_bg_list == NULL){
      return ERROR;
    }

   head_bg_list->prev = NULL;
   head_bg_list->next = NULL;
   bg_list_counter++;
   head_bg_list->status = RUNNING;  /* Running */
   head_bg_list->number = bg_list_counter;
   head_bg_list->pid = proc_id;
   tail_bg_list = head_bg_list;

   /* Allocate memory for the command that started the process*/
   head_bg_list->p_command=malloc(strlen(arr_parsed_command[0]) + 1);

   if (head_bg_list->p_command == NULL){
     return ERROR;
   }
   /* copy the command from the input buffer */

   strncpy(head_bg_list->p_command,arr_parsed_command[0],strlen(arr_parsed_command[0]));
   head_bg_list->p_command[strlen(arr_parsed_command[0])] = '\0'; 

 return OK;
}
/***********************************************/
/* function name : i_set_list_item_done()      */
/* return falue  :                             */
/***********************************************/
/* When the process finish the status var is set to done */
void  i_set_list_item_done(int pid)
{
 struct bg_plist * bg_index = NULL;

 bg_index = head_bg_list; /* Set at  the beggining of  the list*/

 while( ( bg_index != NULL )  && ( bg_index->pid != pid ) ){
      bg_index=bg_index->next;
 }

 if ( bg_index == NULL ){
/*  destructor();*/
  exit(1);
  }

 bg_index->status=DONE; /* Set the status of the process as DONE */

}
/***********************************************/
/* function name : i_child_sig_func()          */
/* return falue  :                             */
/***********************************************/
/* This function is executed  when SIGSHLD is received */
/*void v_child_sig_func(int signal,siginfo_t *sip, void *extra)*/
void v_child_sig_func(int signal)
{
int i_chld_pid = 0;
int i_status = 0 ;

 i_chld_pid=waitpid(0,&i_status,WNOHANG);

 /* wait error*/
 if (i_chld_pid == -1){
     perror("child_sig_func");
     return;
 }
  /* if Bg process Mark it as Done ...else Fg process ..let the shell goes*/
 if( i_bg_list_in_check(i_chld_pid) == ERROR ){
    /* The process is FG - let the shell continue */
     if (sem_post(&sf_fg_process) == -1 ){
         perror("!# sem_wait - exec_thread ");
         exit(1);
     }/*if*/

 } /*if*/
}
/***********************************************/
/* function name : i_child_sig_func()          */
/* return falue  :                             */
/***********************************************/
void v_kill_sig_func(int signal){

 
 if( i_fg_pid != 0){
    kill(i_fg_pid,SIGKILL);
  }/*if*/

  i_fg_pid = 0; 
/*    v_child_sig_func(1); */

}
/* This function initialize the SIGCHLD hook */
/***********************************************/
/* function name : i_child_sig_func()          */
/* return falue  :                             */
/***********************************************/
void init_SIGCHLD_hook(){

struct sigaction sa;

sa.sa_handler=v_child_sig_func;
sigemptyset(&sa.sa_mask);
sa.sa_flags=0;

sa_kill.sa_handler=v_kill_sig_func;
sigaddset(&sa_kill.sa_mask,SIGINT);
sa_kill.sa_flags=0;

   
  if(sigaction(SIGCHLD,&sa,NULL) == -1){
    perror(" init SIGCHLD ");
    v_global_shell_destructor();
    exit(1);
  } 
 
   sigemptyset(&sb); 
   sigaddset(&sb,SIGINT);
   sigaddset(&sb,SIGQUIT);
   sigaddset(&sb,SIGTERM);
   sigaddset(&sb,SIGTSTP);
   sigaddset(&sb_child,SIGCHLD);
   sigprocmask(SIG_BLOCK, &sb, NULL);

  if(sigaction(SIGINT,&sa_kill,NULL) == -1){
    perror(" init SIGINT ");
    v_global_shell_destructor();
    exit(1);
  } 

}


