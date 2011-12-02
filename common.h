#define _POSIX_C_SOURCE 199506L

#define FALSE                   0
#define TRUE                    1

#define OK                      0
#define ERROR                  -1
#define MAXLINE_LEN           512   /* max line size */
#define PARSED_COMMAND_LENGHT  15   /* max number of single string in a single command */



/* threads */

void mutex_cond_init();
int b_pthreads_init();

/* parser.c*/

typedef struct{
 int b_bg_process; /* set it if the process should run in background*/
 int b_pipe; /* set it if there is a pipe in the command */
 char * stdin_;
 char * stdout_;
 int fd_err;
 int fd_input;
 int fd_output;
} proc_info_rec_t;

int b_parser();
void  v_arr_parsed_init();
void v_arr_parsed_destroy();
/*Linked List*/

void v_print_jobs_list(); /* List of the current jobs */
int  i_remove_item_linked_list(); /* Remove all jobs that has status DONE from the list */
int  i_set_up_linked_list(int);
int  i_add_to_end_linked_list(int);
void init_SIGCHLD_hook();
int  i_destroy_linked_list();
/* MAIN*/
void v_global_shell_destructor();

