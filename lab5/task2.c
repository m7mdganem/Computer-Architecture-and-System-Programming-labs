#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include "LineParser.h"

#define TERMINATED  -1
#define RUNNING      1
#define SUSPENDED    0

typedef struct process{
    cmdLine* cmd;                    /* the parsed command line*/
    pid_t pid; 		                 /* the process id that is running the command*/
    int status;                      /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next;	         /* next process in chain */
} process;

void execute(cmdLine *pCmdLine);
void addProcess(process** process_list, cmdLine* cmd, pid_t pid);
void printProcessList(process** process_list);
void freeProcessList(process* process_list);
void updateProcessStatus(process* process_list, int pid, int status);
void updateProcessList(process **process_list);

process *processList = NULL;        /* global variable (The list of processes) */
int debug = 0;                      /* global variable (indecates debugging mode) */

int main(int argc, char **argv){

    // updating program configuration
    for(int i=1; i<argc; i++)
        if(strncmp(argv[i],"-d",2)==0) debug = 1;

    // get current working directory path
    char path[PATH_MAX];
    getcwd(path,PATH_MAX);

    int printdirectory = 1; /* only a boolean that can help us not to print the directory path at the beggining of the while after suspend */

    while (1){

        fprintf(stderr,"%s> ",path);

        // read user's input
        char userLine[2048];
        fgets(userLine, 2048, stdin);

        // parse the user's input line
        cmdLine *pCmdLine = parseCmdLines(userLine);

        // execute the command, fork if it is needed
        if(strcmp(userLine, "\n") == 0)
            continue;
        else if(strcmp(pCmdLine->arguments[0], "quit") == 0){
            freeCmdLines(pCmdLine);
            break;
        }else if(strcmp(pCmdLine->arguments[0], "cd") == 0){
            if(chdir(pCmdLine->arguments[1]) != 0)
                perror("Error executing");
            else
                getcwd(path,PATH_MAX);
            freeCmdLines(pCmdLine);
            continue;
        }else if(strcmp(pCmdLine->arguments[0], "procs") == 0){
            printProcessList(&processList);
            freeCmdLines(pCmdLine);
            continue;
        }else{
            execute(pCmdLine);
        }        
    }

    // free processes list
    freeProcessList(processList);

	return 0;
}

void execute(cmdLine *pCmdLine){
    int cpid;
    if( (cpid = fork()) ){ /* parent process */
        if(strcmp(pCmdLine->arguments[0], "suspend")==0 || strcmp(pCmdLine->arguments[0],"kill")==0){
            freeCmdLines(pCmdLine);
            return;
        }
        addProcess(&processList,pCmdLine,cpid); /* parent adds process to the list */
    }else{ /* child process */
        if(debug)
            fprintf(stderr, "PID: %d\nExecuting command\n", getpid());
        if(strcmp(pCmdLine->arguments[0], "suspend") == 0){
            int pidToKill = atoi(pCmdLine->arguments[1]);
            if(kill(pidToKill,SIGTSTP) != 0){
                perror("Error");
                _exit(1); //error
            }
            sleep(atoi(pCmdLine->arguments[2]));
            if(kill(pidToKill,SIGCONT) != 0){
                perror("Error");
                _exit(1); //error
            }
            freeProcessList(processList);
            freeCmdLines(pCmdLine);
            exit(0);
        }else if(strcmp(pCmdLine->arguments[0], "kill") == 0){
            if(kill(atoi(pCmdLine->arguments[1]),SIGINT) != 0){
                perror("Error sending SIGINT");
                _exit(1); //error
            }
            freeProcessList(processList);
            freeCmdLines(pCmdLine);
            exit(0);
        }
        execvp(pCmdLine->arguments[0], pCmdLine->arguments);
        perror("Error executing");
        freeProcessList(processList);
        freeCmdLines(pCmdLine);
        _exit(1); //error
    }
    // wait for child if the command is blocking
    if(pCmdLine->blocking == 1)
        waitpid(cpid,NULL,0);
}
void freeProcessList(process* process_list){
    if(!process_list) 
        return;
    freeCmdLines(process_list->cmd);
    if(process_list->next) 
        freeProcessList(process_list->next);
    free(process_list);
}

void addProcess(process** process_list, cmdLine* cmd, pid_t pid){
    process *newProcess = (process *) malloc(sizeof(process));
    newProcess->cmd = cmd;
    newProcess->pid = pid;
    newProcess->status = RUNNING;
    newProcess->next = *process_list;
    *process_list = newProcess;
}

void updateProcessStatus(process* process_list, int pid, int status){
    process *p = process_list;
    if(!p) return;
    while(p && p->pid != pid)
        p = p->next;
    if(p)
        p->status = status;
}

void updateProcessList(process **process_list){
    process *p = *process_list;
    while(p){
        int pid = p->pid;
        int status;
        int ans = waitpid(pid,&status,WNOHANG|WCONTINUED|WUNTRACED);
        if(ans == pid && WIFCONTINUED(status)) /* Check if the process had been continued */
            updateProcessStatus(processList,p->pid,RUNNING);
        else if(ans == pid && WIFSTOPPED(status)) /* Check if the process had been stopped */
            updateProcessStatus(processList,p->pid,SUSPENDED);
        else if(ans == pid || ans == -1) /* Check if the process had been terminated */
            updateProcessStatus(p,pid,TERMINATED);
        p = p->next;
    }
}

void printProcessList(process** process_list){
    if(!process_list) return; // if the first element in the list is null
    process *p = *process_list;
    process *prev = NULL;
    updateProcessList(process_list);
    fprintf(stderr, "%-15s %-15s %s\n", "PID", "Command", "STATUS");
    while(p){
        fprintf(stderr, "%-15d %-15s %s\n", p->pid, p->cmd->arguments[0], p->status == TERMINATED  ?  "TERMINATED" :
                                                                          p->status ==  SUSPENDED  ?  "SUSPENDED" :
                                                                                                      "RUNNING");
        if(p->status == TERMINATED){
            if(p == *process_list){
                process *tmp = p->next;
                p->next = NULL;
                *process_list = tmp;
                freeProcessList(p);
                p = tmp;
                continue;
            }else{
                prev->next = p->next;
                p->next = NULL;
                freeProcessList(p);
                p = prev->next;
                continue;
            }
        }
        prev = p;
        p = p->next;
    }
}