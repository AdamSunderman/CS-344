// Written By Adam Sunderman for cs344
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>

//---------------------------------------------------------------------------------------------------------------------
//----------------------SPECIAL TYPES AND VALUES-----------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------

//max backgroung processes allowed at once
#define MAX_BG_PROCS 200
//command buffer size
#define BUFFER_SIZE 2048
//max number of arguments allowed in a command
#define ARG_SIZE 512
//setup boolean values for program status flags
typedef enum{FALSE, TRUE} boolean;

//----------------------------------------------------------------------------------------------------------------------
//----------------------FUNCTION DECLARATIONS---------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

//FUNCTION: MallocCheck(), Simple function to test if a memory allocation failed
//   PARAM: char* newvar, Pointer to new memory block 
// RETURNS: void
void MallocCheck(char* newvar);

//FUNCTION: ClearCmdArgs(), Clears all previous command arguments
//   PARAM: char** cmd_args, Double Pointer to array of char* that holds current commands for execvp()
// RETURNS: void 
void ClearCmdArgs(char** cmd_args);

//FUNCTION: ClearBuffers(), Clears all the program buffers
//   PARAM: char* my_buf, Pointer to char array that holds raw unparsed user input
//   PARAM: char* in_file, Pointer to char array that holds the name of any input files
//   PARAM: char* out_file, Pointer to char array that holds the name of any output files
// RETURNS: void 
void ClearBuffers(char* my_buf, char* in_file, char* out_file);

//FUNCTION: ResetFlags(), Clears all the program flags
//   PARAM: boolean* bg, Pointer to flag indicating if the current command is to be ran in the background
//   PARAM: boolean* redir_in, Pointer to flag indicating if the current command is to have it's input redirected
//   PARAM: boolean* redir_out, Pointer to flag indicating if the current command is to have it's output redirected
// RETURNS: void 
void ResetFlags(boolean* bg, boolean* redir_in, boolean* redir_out);

//FUNCTION: GetCmd(), Gets and parses user command. Also sets all program flags and fills all needed buffers
//   PARAM: char* my_buf, Pointer to char array that holds raw unparsed user input
//   PARAM: boolean* bg, Pointer to flag indicating if the current command is to be ran in the background
//   PARAM: boolean* redir_in, Pointer to flag indicating if the current command is to have it's input redirected
//   PARAM: char* in_file, Pointer to char array that holds the name of any input files
//   PARAM: boolean* redir_out, Pointer to flag indicating if the current command is to have it's output redirected
//   PARAM: char* out_file, Pointer to char array that holds the name of any output files
// RETURNS: void 
void GetCmd(char* my_buf, boolean* bg, boolean* redir_in, char* in_file, boolean* redir_out, char* out_file);

//FUNCTION: IsInternalCmd(), Simple function to test if a command is internal or external
//   PARAM: char* my_buf, Pointer to char array that holds raw unparsed user input 
// RETURNS: TYPEDEF boolean, TRUE or FALSE
boolean IsInternalCmd(char* my_buf);

//FUNCTION: IsComment(), Simple function to test if a command is a comment
//   PARAM: char* my_buf, Pointer to char array that holds raw unparsed user input 
// RETURNS: TYPEDEF boolean, TRUE or FALSE
boolean IsComment(char* my_buf);

//FUNCTION: IsBlank(), Simple function to test if a command is blank
//   PARAM: char* my_buf, Pointer to char array that holds raw unparsed user input 
// RETURNS: TYPEDEF boolean, TRUE or FALSE
boolean IsBlank(char* my_buf);

//FUNCTION: RunInternal(), Handles any commands that are built in (cd, status, exit)
//   PARAM: char* my_buf, Pointer to char array that holds raw unparsed user input
//   PARAM: boolean* done, Pointer to flag indicating if the command was to exit the smallsh
//   PARAM: int* bg_procs, Pointer to any running background process ids
//   PARAM: int* bg_proc_count, Pointer to number of current backgrounds processes 
//   PARAM: int* cmd_status, Pointer to the exit status number of the last command that was run
// RETURNS: void 
void RunInternal(char* my_buf, boolean *done, int* bg_procs, int* bg_proc_count, int* cmd_status);

//FUNCTION: SetupExternalCmd(), Parses user input to setup an array of arguments that will be passed to execvp()
//   PARAM: char* my_buf, Pointer to char array that holds raw unparsed user input
//   PARAM: char** cmd_args, Double Pointer to array of char* that holds current commands for execvp()
//   PARAM: int* num_args, Pointer to number of arguments in the current command
// RETURNS: char* A pointer to the last memory address in the array that was set. It gets set to null for execvp()
//                but this address will be replaced later so the memory can be freed
char* SetupExternalCmd(char* my_buf, char** cmd_args, int* num_args);

//FUNCTION: RunExternal(), Handles fork() and exec() for external commands. Does any needed input and output redirection
//                         also sets signal handelers for both parent and child.
//   PARAM: boolean* bg, Pointer to flag indicating if the current command is to be ran in the background
//   PARAM: boolean* redir_in, Pointer to flag indicating if the current command is to have it's input redirected
//   PARAM: char* in_file, Pointer to char array that holds the name of any input files
//   PARAM: boolean* redir_out, Pointer to flag indicating if the current command is to have it's output redirected
//   PARAM: char* out_file, Pointer to char array that holds the name of any output files
//   PARAM: int* bg_procs, Pointer to any running background process ids
//   PARAM: int* bg_proc_count, Pointer to number of current backgrounds processes 
//   PARAM: char** cmd_args, Double Pointer to array of char* that holds current commands for execvp()
//   PARAM: int* cmd_status, Pointer to the exit status number of the last command that was run
// RETURNS: void
void RunExternal(boolean* bg, boolean* redir_in, char* in_file, boolean* redir_out, char* out_file, int* bg_procs, int *bg_proc_count,  char** cmd_args, int *cmd_status);

//FUNCTION: BgScan(), Scan for any completed background processes and either prints and exit or termination code
//   PARAM: int* bg_procs, Pointer to any running background process ids
//   PARAM: int* bg_proc_count, Pointer to number of current backgrounds processes 
// RETURNS: void
void BgScan(int* bg_procs, int bg_proc_count);

//FUNCTION: BgKill(), Kills all background processes if a user chooses the internal command 'exit'
//   PARAM: int* bg_procs, Pointer to any running background process ids
//   PARAM: int* bg_proc_count, Pointer to number of current backgrounds processes 
// RETURNS: void
void BgKill(int* bg_procs, int* bg_proc_count);

//-------------------------------------------------------------------------------------------------------------
//------------------------------------------MAIN---------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[]){

	// flag for input redirection
    boolean redirect_in = FALSE;
    // input redirection filename
    char* in_file = malloc(ARG_SIZE);
    MallocCheck(in_file);

    // flag for output redirection
    boolean redirect_out = FALSE;
    // output redirection filename
    char* out_file = malloc(ARG_SIZE);
    MallocCheck(out_file);

    // flag for if current command is a background command
    boolean bg_proc = FALSE;
    // holds commands
    char* cmd_buffer = malloc(BUFFER_SIZE);
    MallocCheck(cmd_buffer);

    // holds background pids
    int bg_procs[MAX_BG_PROCS];
    int bg_proc_count = 0;

    // holds exit or term status of last command
    int cmd_status = 0;

    // main program loop flag only set to true when user chooses 'exit'
    boolean exit_smallsh = FALSE;
    while(exit_smallsh == FALSE){
    	// Ignore interupts
        signal(SIGINT, SIG_IGN);

        // setup array and counter for current cmd arguments 
        char* cmd_args[ARG_SIZE];
        int i;
        for(i = 0; i < ARG_SIZE; i = i +1){
            cmd_args[i] = malloc(75);
        }
        int num_args = 0;

        // when an array of cmd_args is built the last pointer gets set to null
        // this holds the address so it can be freed later
        char* arg_end;

        // check if any background processes are done
        BgScan(bg_procs, bg_proc_count);
        // clear program buffers
        ClearBuffers(cmd_buffer, in_file, out_file);
        // reset all program flags
        ResetFlags(&bg_proc, &redirect_in, &redirect_out);
        // reset the array of command arguments
        ClearCmdArgs(cmd_args);
        // get the next command
        GetCmd(cmd_buffer, &bg_proc, &redirect_in, in_file, &redirect_out, out_file);

        // ignore comments
        if(IsComment(cmd_buffer) == TRUE){
            continue;
        }
        // ignore blank lines
        else if(IsBlank(cmd_buffer) == TRUE){
            continue;
        }
        // run internal commands
        else if(IsInternalCmd(cmd_buffer) == TRUE){
            RunInternal(cmd_buffer, &exit_smallsh, bg_procs , &bg_proc_count, &cmd_status);
            for(i = 0; i < ARG_SIZE; i = i +1){
                free(cmd_args[i]);
            }
        }
        //run external commands
        else{
        	// setup and run the command then free the command argument array
            arg_end = SetupExternalCmd(cmd_buffer, cmd_args, &num_args);
            RunExternal(&bg_proc, &redirect_in, in_file, &redirect_out, out_file, bg_procs , &bg_proc_count, cmd_args, &cmd_status);
            cmd_args[num_args] = arg_end;
            for(i = 0; i < ARG_SIZE; i = i +1){
                free(cmd_args[i]);
            }
        }
    }
    // deal with memory
    free(cmd_buffer);
    free(in_file);
    free(out_file);
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------FUNCTION DEFINITIONS--------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------

void MallocCheck(char *newvar){
    if(newvar == NULL){
        perror("malloc failed");
        exit(1);
    }
}

void ClearCmdArgs(char** cmd_args){
    int i;
    for(i = 0; i < ARG_SIZE; i = i + 1){
        strcpy(cmd_args[i], "");
    }

}

void ClearBuffers(char* my_buf, char* in_file, char* out_file){
    strcpy(my_buf, "");
    strcpy(in_file, "");
    strcpy(out_file, "");
}

void ResetFlags(boolean* bg, boolean* redir_in, boolean* redir_out){
    *bg = FALSE;
    *redir_in = FALSE;
    *redir_out = FALSE;
}

void GetCmd(char* my_buf,  boolean* bg , boolean *redir_in, char * in_file, boolean *redir_out, char * out_file){
   	
   	// index variables for parsing user input
    char* index1;
    char* index2;

    //get the command
    fflush(stdout);
    printf(": ");
    fflush(stdout);
    char cmd[BUFFER_SIZE];
    fgets(cmd, sizeof(cmd), stdin);

    // makes sure there is valid input
    if( cmd != NULL && IsComment(cmd) != TRUE){

    	// get rid of newline
        index1 = strchr(cmd, '\n');
        *index1 = '\0';

        // check if it a backgorund command
        index1 = strrchr(cmd, '&');
        if( index1 != NULL ){
            *bg = TRUE;
            *index1 = '\0';
            index1 = strrchr(cmd, ' ');
            *index1 = '\0';
        }

        //check for input and output redirection
        index1 = strrchr(cmd, '>');
        index2 = strrchr(cmd, '<');
        // if there is redirection
        if( index1 != NULL || index2 != NULL){

        	// determine order and number of redirections

        	// if there are both and input redirection comes first in the string or if just output redirection 
            if( index1 > index2 || index2 == NULL){
                *redir_out = TRUE;
                index1 = index1 + (sizeof(char) * 2);
                strncpy(out_file, index1 , strlen(index1));
                index1 = index1 - (sizeof(char) * 2);
                *index1 = '\0';
                index1 = strrchr(cmd, ' ');
                *index1 = '\0';
                // if there are both
                if( index2 != NULL ){
                    *redir_in = TRUE;
                    index2 = index2 + (sizeof(char) * 2);
                    strncpy(in_file, index2, strlen(index2));
                    index2 = index2 - (sizeof(char) * 2);
                    *index2 = '\0';
                    index2 = strrchr(cmd, ' ');
                    *index2 = '\0';
                }
            }
            // if there are both or output redirection comes first in the string
            else{
                *redir_in = TRUE;
                index2 = index2 + (sizeof(char) * 2);
                strncpy(in_file, index2, strlen(index2));
                index2 = index2 - (sizeof(char) * 2);
                *index2 = '\0';
                index2 = strrchr(cmd, ' ');
                *index2 = '\0';
                // if there are both
                if( index1 != NULL ){
                    *redir_out = TRUE;
                    index1 = index1 + (sizeof(char) * 2);
                    strncpy(out_file, index1 , strlen(index1));
                    index1 = index1 - (sizeof(char) * 2);
                    *index1 = '\0';
                    index1 = strrchr(cmd, ' ');
                    *index1 = '\0';
                }
            }
        }
    }
    // command is now ready 
    strcpy(my_buf, cmd);
}

boolean IsInternalCmd(char* my_buf){
    if(strncmp(my_buf, "status", strlen("status")) == 0 || strncmp(my_buf, "cd", strlen("cd")) == 0 || strncmp(my_buf, "exit", strlen("exit")) == 0){
        return TRUE;
    }
    else{
        return FALSE;
    }
}

boolean IsComment(char* my_buf){
    if(strncmp(my_buf, "#",1) == 0){
        return TRUE;
    }
    else{
        return FALSE;
    }
}

boolean IsBlank(char* my_buf){
    if(strcmp(my_buf, "") == 0){
        return TRUE;
    }
    else{
        return FALSE;
    }
}

void RunInternal(char* my_buf, boolean *done, int* bg_procs, int* bg_proc_count, int* cmd_status){
   	// run 'status' command
    if(strncmp(my_buf, "status", strlen("status")) == 0){
        printf("exited with status: %d\n", *cmd_status);
        fflush(stdout);
    }
    // run cd command
    else if(strncmp(my_buf, "cd", strlen("cd")) == 0){
    	// just cd
        if(strcmp(my_buf, "cd") == 0){
            char *home = getenv("HOME");
            chdir(home);
        }
        // build path
        else{
            char cwd[ARG_SIZE];
            getcwd(cwd, sizeof(cwd));
            char* path = strstr(my_buf, " ");
            if(path) {
                path = path + 1;
                char *value;
                value = malloc(strlen(path));
                memcpy(value, path, strlen(path));
                *(value + strlen(path)) = 0;
                sprintf(cwd, "%s/%s", cwd, value);
                free(value);
            }
            chdir(cwd);
        }
    }
    // run exit command
    else if(strncmp(my_buf, "exit", strlen("exit")) == 0){
        *done = TRUE;
        BgKill(bg_procs, bg_proc_count);
    }
}

char* SetupExternalCmd(char* my_buf,  char** cmd_args, int* num_args){
	// break up arguments for external command
    char* token;
    token = strtok(my_buf," ");
    do{	
    	// if there is expansion needed on any files named using pid
        int expand = strcspn(token, "$");
        if(expand != strlen(token)){
            char temp[ARG_SIZE];
            strncpy(temp, token, expand);
            char expansion[ARG_SIZE];
            sprintf(expansion, "%ld", (long)getpid());
            strcat(temp,expansion);
            strcpy(cmd_args[*num_args], temp);
            *num_args = *num_args + 1;
            token = strtok(NULL, " ");
            continue;
        }
        // else just save to argument array
        strcpy(cmd_args[*num_args], token);
        *num_args = *num_args + 1;
        token = strtok(NULL, " ");
    }while(token != NULL);

    // save the address that we need to set to null for passing to exec()
    char* end = cmd_args[*num_args];
    cmd_args[*num_args] = NULL;

    return end;
}

void RunExternal(boolean* bg, boolean* redir_in, char* in_file, boolean* redir_out, char* out_file, int* bg_procs, int* bg_proc_count,  char** cmd_args, int* cmd_status){

    int failed_exec;
    // fork a child
    pid_t spawn_pid = -5;
    spawn_pid = fork();
    if(spawn_pid < 0){
        perror("Fork Error");
        fflush(stdout);
        exit(1);
    }

    //if this is the child
    if(spawn_pid == 0){

    	// redirect the input
        if(*redir_in == TRUE){
            int fd_in = open(in_file, O_RDONLY, 0);
            if(fd_in < 0){
                perror("Could not open input file");
                fflush(stdout);
                exit(1);
            }
            else{
                dup2(fd_in, STDIN_FILENO);
            }
        }

        // redirect the output
        if(*redir_out == TRUE){
            int fd_out = creat(out_file, 0644);
            if(fd_out < 0){
                perror("Could not open output file");
                fflush(stdout);
                exit(1);
            }
            else{
                dup2(fd_out, STDOUT_FILENO);
            }
        }

        // run in the background
        if(*bg == TRUE){
        	// if there was no redirection send it all to /dev/null
            if(*redir_out == FALSE){
                int fd_out2 = open("/dev/null", O_WRONLY);
                dup2(fd_out2, STDOUT_FILENO);
            }
            if(*redir_in == FALSE){
                int fd_in2 = open("/dev/null", O_RDONLY);
                dup2(fd_in2 , STDIN_FILENO);
            }

            // run the command with execvp()
            failed_exec = execvp(cmd_args[0], cmd_args);
            if(failed_exec < 0){
                perror("exec() failed");
                fflush(stdout);
                exit(1);
            }
        }
        // run in the foreground
        else{
            signal(SIGINT, SIG_DFL);
            failed_exec = execvp(cmd_args[0], cmd_args);
            if(failed_exec < 0){
                perror("exec() failed");
                fflush(stdout);
                exit(1);
            }
        }

    }

    // if this is the parent
    else{

    	// if it was a background command save the process id and leave
        if(*bg == TRUE){
            printf("background pid is %d\n", spawn_pid);
            fflush(stdout);
            bg_procs[*bg_proc_count] = spawn_pid;
            *bg_proc_count = *bg_proc_count + 1;
        }

        // or wait for the process
        else{
            int fg_status;
            waitpid(spawn_pid, &fg_status, 0);

            // if exited 
            if(WIFEXITED(fg_status)) {
                *cmd_status = WEXITSTATUS(fg_status);
            }

            // if signaled
            if(WIFSIGNALED(fg_status)) {
                *cmd_status = WTERMSIG(fg_status);
                printf("terminated by signal %d", *cmd_status);
                fflush(stdout);
            }
        }
    }
}

void BgScan(int* bg_procs, int bg_proc_count){

    int status;
    int val;
    int i;

    // loop through active processes, check if and how they terminated 
    for(i = 0; i < bg_proc_count; i = i + 1) {
        if(waitpid(bg_procs[i], &status, WNOHANG) > 0) {

        	// if exited
            if(WIFEXITED(status)) {
                val = WEXITSTATUS(status);
                printf("background pid %d is done: exit value %d\n", bg_procs[i], val);
                fflush(stdout);
                bg_proc_count = bg_proc_count -1;
            }
            
            // if signaled
            if(WIFSIGNALED(status)) {
                val = WTERMSIG(status);
                printf("background pid %d is done: terminated by signal %d\n", bg_procs[i], val);
                fflush(stdout);
                bg_proc_count = bg_proc_count -1;
            }
        }
    }
}

void BgKill(int* bg_procs, int* bg_proc_count){
    int i;
    for(i = 0; i < *bg_proc_count; i = i + 1) {
        kill((bg_procs[i]), SIGKILL);
    }
}

