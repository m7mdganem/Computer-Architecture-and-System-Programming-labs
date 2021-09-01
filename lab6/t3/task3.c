#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include "LineParser.h"

#define maxNumOfCmds 10

void execute(cmdLine *pCmdLine);
void redirectIO(cmdLine *parsedLine, int pipeDesc[]);

int debug = 0;                      /* global variable (indecates debugging mode) */
int pipeBool;                       /* boolean indicates piping */

int main(int argc, char **argv){

    // updating program configuration
    for(int i=1; i<argc; i++)
        if(strncmp(argv[i],"-d",2)==0) debug = 1;

    // get current working directory path
    char path[PATH_MAX];
    getcwd(path,PATH_MAX);
    char *history[maxNumOfCmds] = {NULL};
    int currentHistoryWriteIndex = 0;
    int numberOfCommands = 0;
    while (1){
        pipeBool = 0; 

        fprintf(stderr,"%s> ",path);
        // read user's input
        char *userLine = (char *)malloc(2048 * sizeof(char));
        fgets(userLine, 2048, stdin);
        if(strncmp(userLine,"!",1)==0){
            char ind[2];
            strncpy(ind,userLine+1,1);
            int index = atoi(ind);
            if(index >= numberOfCommands || index < 0){
                fprintf(stderr,"Invalid history index\n");
                continue;
            }
            userLine = history[index];
        }
        // parse the user's input line
        cmdLine *pCmdLine = parseCmdLines(userLine);
        if(!strcmp(pCmdLine->arguments[0], "history") == 0){
            if(history[currentHistoryWriteIndex % 10])
                free(history[currentHistoryWriteIndex % 10]);
            history[currentHistoryWriteIndex % 10] = userLine;
            currentHistoryWriteIndex++;
            if(numberOfCommands < 10)
                numberOfCommands++;
        }
        if(pCmdLine->next)
            pipeBool = 1;
        // execute the command, fork if it is needed
        int cpid;
        if(strcmp(pCmdLine->arguments[0], "quit") == 0){
            freeCmdLines(pCmdLine);
            break;
        }else if(strcmp(pCmdLine->arguments[0], "cd") == 0){
            if(chdir(pCmdLine->arguments[1]) != 0)
                perror("chdir");
            else
                getcwd(path,PATH_MAX);
            freeCmdLines(pCmdLine);
            continue;
        }else if(strcmp(pCmdLine->arguments[0], "history") == 0){
            for (size_t i = 0; i < numberOfCommands ; i++){
                fprintf(stderr, "%d- %s",i, history[i]);
            }         
            freeCmdLines(pCmdLine);
        }else{
            execute(pCmdLine);
            freeCmdLines(pCmdLine);
        }
    }
    
    for (size_t i = 0; i < numberOfCommands; i++){
        free(history[i]);
    }
	return 0;
}

void execute(cmdLine *pCmdLine){
    int pipeDiscs[2]; /* pipedes[0] = read-end; pipedes[1] = write-end; */
    if(pipeBool)
        if(pipe(pipeDiscs) == -1){
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    int cpid  = fork();
    if(cpid == -1){
        perror("first fork");
        exit(EXIT_FAILURE);
    }
    if (!cpid){  /* child process*/
        if(debug) fprintf(stderr, "PID: %d\nExecuting command\n", getpid());
        redirectIO(pCmdLine,pipeDiscs); /* redirections */
        execvp(pCmdLine->arguments[0], pCmdLine->arguments);
        perror("Executing first command");
        freeCmdLines(pCmdLine);
        _exit(EXIT_FAILURE); //error
    }
    if(pipeBool) close(pipeDiscs[1]);
    int c2pid;
    cmdLine *secondCommand = pCmdLine->next;
    if(pipeBool){
        c2pid = fork();
        if(c2pid == -1){
            perror("second fork");
            exit(EXIT_FAILURE);
        }
        if(!c2pid){
            if(debug) fprintf(stderr, "PID: %d\nExecuting command\n", getpid());
            redirectIO(secondCommand,pipeDiscs); /* redirections */
            execvp(secondCommand->arguments[0],secondCommand->arguments);
            perror("Executing second command");
            freeCmdLines(secondCommand);
            _exit(EXIT_FAILURE); //error
        }
    }
    /* Close unneeded files and wait for children*/
    if(pipeBool) close(pipeDiscs[0]);
    if(cpid > 0) waitpid(cpid,NULL,0);
    if(c2pid > 0) waitpid(c2pid,NULL,0);
    
}

void redirectIO(cmdLine *parsedLine, int pipeDesc[]){
    if(parsedLine->inputRedirect){
        close(0);
        if(!fopen(parsedLine->inputRedirect,"r")){
            perror("Input redirection");
            exit(1);
        }
    }else if(pipeBool && !parsedLine->next){ /* pipe and it is the last cmd line  */
        close(0);
        close(pipeDesc[1]);
        int newOutput = dup(pipeDesc[0]);
        close(pipeDesc[0]);
    }
    if(parsedLine->outputRedirect){
        close(1);
        if( !fopen(parsedLine->outputRedirect,"w")){
            perror("Output redirection");
            exit(1);
        }
    }else if(pipeBool && parsedLine->next){ /* pipe and not the last cmd line  */
        close(1);
        close(pipeDesc[0]);
        int newOutput = dup(pipeDesc[1]);
        close(pipeDesc[1]);
    }
}