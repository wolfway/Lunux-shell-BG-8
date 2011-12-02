#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


proc_info_rec_t proc_info_rec;

extern char arr_input_buff[MAXLINE_LEN + 1];
extern char * arr_parsed_command[PARSED_COMMAND_LENGHT];
/***********************************************/
/* function name : v_arr_parsed_command()      */
/* return value  :                             */
/***********************************************/
void  v_arr_parsed_init(){
int i = 0;
  
  for(i=0;i<PARSED_COMMAND_LENGHT;i++)
    arr_parsed_command[i]= NULL; 
     
}
/***********************************************/
/* function name : v_arr_parsed_destroy()      */
/* return value  :                             */
/***********************************************/
void  v_arr_parsed_destroy(){

int i =0;

 while((arr_parsed_command[i]!= NULL) && (i< PARSED_COMMAND_LENGHT) ){
  free(arr_parsed_command[i]);
   arr_parsed_command[i]=NULL;
  i++;
 }

}
/***********************************************/
/* function name : v_shift_pointer()           */
/* return value  :                             */
/***********************************************/
void v_shift_pointers(int start, int  end){
int i = 0;

 for(i=start;i<end;i++)
  arr_parsed_command[i]=arr_parsed_command[i+1];

  arr_parsed_command[end]=NULL;

}
/***********************************************/
/* function name : v_parser()                  */
/* return value  :                             */
/***********************************************/
int b_parser(){
 int i = 0;
 int index_start=0;
 int index_end=0;
 int index_par_command=0;

    v_arr_parsed_destroy();/* release the buffers allocated for the prevous command*/
    while( index_par_command < PARSED_COMMAND_LENGHT){ 

   while( arr_input_buff[index_end]==' '){/* remove the spaces at the begining and between the parameteres*/
     index_end++;
   }/*while*/


   if( arr_input_buff[index_end]=='\n'){
     break;
   }/*if*/
   index_start = index_end;

   while( (arr_input_buff[index_end]!=' ') && (arr_input_buff[index_end]!='\n')){
    index_end++;
    }/*while*/
   if ( (arr_parsed_command[index_par_command]=(char *)malloc( index_end - index_start +1)) == NULL  ){
      perror("parse_in_buffer");
      return(ERROR);
   } /*if*/

  /* Copy the command string from the input Buffer to the Parser buffer */
  strncpy(arr_parsed_command[index_par_command],&arr_input_buff[index_start],index_end - index_start);  
 /* Terminat the string*/
  arr_parsed_command[index_par_command][index_end-index_start]='\0'; 
  
  index_par_command++;
 }/*while */




 /* The were more strings than the limit */
 if( index_par_command == PARSED_COMMAND_LENGHT ){ 
   printf("!# Max number of commands strings is %d ! \n",PARSED_COMMAND_LENGHT); 
   printf("!# All others were ignored  \n");
 }

 /* Decrement the number of the commands  to the right number  */
     index_par_command--; 

 /* Init the proc_info_rec Structure */
 memset(&proc_info_rec,0,sizeof(proc_info_rec));

 /* Fill the proc_info_rec structure */
 for( i=0; i<=index_par_command;i++){
   /* Look for | >< or &*/
   if(strlen(arr_parsed_command[i]) == 1){
       /*&*/ 
       if(strncmp(arr_parsed_command[i],"&",1) == OK ){
          if (i > 0){ /* command missing */  
              proc_info_rec.b_bg_process = TRUE;
              v_shift_pointers(i,index_par_command);
              index_par_command--;
              i--;
              continue;
         }/*if*/
         else{
             printf("!# Command Missing \n");
             return ERROR;
         }/*else*/
       }/*if*/
   
       /* > redirect  output*/
       if(strncmp(arr_parsed_command[i],">",1) == OK ){
           if(arr_parsed_command[i+1] != NULL && i > 0){/* i > 0  < is not first string - command missing */
             /*Free this memory in v_shell_exec_thread !!!!*/  
             proc_info_rec.stdout_ = malloc(strlen(arr_parsed_command[i+1]) + 1);
             strcpy(proc_info_rec.stdout_,arr_parsed_command[i+1]);
             proc_info_rec.stdout_[strlen(arr_parsed_command[i+1])] = '\0';
             /* remove the > file /divece from the string */
             v_shift_pointers(i,index_par_command);
             index_par_command--;
             i--;
             v_shift_pointers(i+1,index_par_command);
             index_par_command--;  
             i--;
            continue;
           }/*if*/
           else{
                 printf("!# >  should be followed by file or divece !!! or no command \n");
                 return ERROR;
           }/*else*/ 
        }/*if > */
     
        /*< redirect input */
       if(strncmp(arr_parsed_command[i],"<",1) == OK ){
           if(arr_parsed_command[i+1] != NULL && i > 0){/* i > 0  < is not first string - command missing */
              proc_info_rec.stdin_ = malloc(strlen(arr_parsed_command[i+1]) + 1);
              strcpy(proc_info_rec.stdin_,arr_parsed_command[i+1]);
              proc_info_rec.stdin_[strlen(arr_parsed_command[i+1])] = '\0';
              /* remove the < file /divece from the string */
              v_shift_pointers(i,index_par_command);
              index_par_command--;
              i--;
              v_shift_pointers(i+1,index_par_command);
              index_par_command--;         
              i--; 
              continue;
            }/*if*/
            else{
                 printf("!# <  should be followed by file or divece !!! or no command\n ");
                return ERROR;
           }/*else*/
       }/*if*/
   
         /* | pipe - not supported */
       if(strncmp(arr_parsed_command[i],"|",1) == OK ){
           printf(" PIPES are not SUPPORTED !!! ");
           return ERROR;
       }/*if*/
   }/* if lenght = 1*/    
 }/*for*/
 

return OK;
}

