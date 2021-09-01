#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include "LineParser.h"

void execute(cmdLine *pCmdLine);

int debug = 0;                      /* global variable (indecates debugging mode) */

int main(int argc, char **argv){

    // updating program configuration
    for(int i=1; i<argc; i++)
        if(strncmp(argv[i],"-d",2)==0) debug = 1;

    // get current working directory path
    char path[PATH_MAX];
    getcwd(path,PATH_MAX);

    while (1){
        fprintf(stderr,"%s> ",path);
        // read user's input
        char userLine[2048];
        fgets(userLine, 2048, stdin);

        // parse the user's input line
        cmdLine *pCmdLine = parseCmdLines(userLine);

        // execute the command, fork if it is needed
        int cpid;
        if(strcmp(pCmdLine->arguments[0], "quit") == 0){
            freeCmdLines(pCmdLine);
            break;
        }else if(strcmp(pCmdLine->arguments[0], "cd") == 0){
            if(chdir(pCmdLine->arguments[1]) != 0)
                perror("Error");
            else
                getcwd(path,PATH_MAX);
            freeCmdLines(pCmdLine);
            continue;
        }else{
            execute(pCmdLine);
            freeCmdLines(pCmdLine);
        }
    }
	return 0;
}

void execute(cmdLine *pCmdLine){
    int cpid;
    if ( !(cpid = fork()) ){
        if(debug)
            fprintf(stderr, "PID: %d\nExecuting command\n", getpid());
        execvp(pCmdLine->arguments[0], pCmdLine->arguments);
        perror("Error");
        freeCmdLines(pCmdLine);
        _exit(1); //error
    }else{
        if(pCmdLine->blocking == 1)
            waitpid(cpid,NULL,0);
    }
}