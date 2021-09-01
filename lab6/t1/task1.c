#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char const *argv[]){

    // updating program configuration
    int debug = 0;
    for(int i=1; i<argc; i++)
        if(strncmp(argv[i],"-d",2)==0) debug = 1;

    int pipeDesc[2];
    if(pipe(pipeDesc) == -1){ /* pipeDesc[0] = read-end; pipeDesc[1] = write-end; */
        perror("ERROR");
        exit(1);
    }
    if(debug) fprintf(stderr,"(parent process>forking...)\n");
    pid_t child1pid = fork();
    if(debug && child1pid) fprintf(stderr,"(parent process>created process with id: %d)\n",child1pid);
    if(child1pid < 0){
        perror("Forking child 1");
        exit(1);
    }
    if(!child1pid){
        if(debug) fprintf(stderr,"(child1>redirecting stdout to the write end of the pipe...)\n");
        close(1);
        int newOutput = dup(pipeDesc[1]);
        close(pipeDesc[1]);
        char *arguments[3] = {"ls", "-l", '\0'};
        if(debug) fprintf(stderr,"(child1>going to execute cmd: ...)\n");
        execvp(arguments[0], arguments);
        perror("Executing ls");
        exit(1);
    }
    if(debug) fprintf(stderr,"(parent process>closing the write end of the pipe...)\n");
    close(pipeDesc[1]);
    if(debug) fprintf(stderr,"(parent process>forking...)\n");
    pid_t child2pid = fork();
    if(debug && child2pid) fprintf(stderr,"(parent process>created process with id: %d)\n",child2pid);
    if(child2pid < 0){
        perror("Forking child 2");
        exit(1);
    }
    if(!child2pid){
        if(debug) fprintf(stderr,"(child2>redirecting stdout to the write end of the pipe...)\n");
        close(0);
        int newInput = dup(pipeDesc[0]);
        close(pipeDesc[0]);
        char *arguments[4] = {"tail", "-n", "2", '\0'};
        if(debug) fprintf(stderr,"(child2>going to execute cmd: ...)\n");
        execvp(arguments[0],arguments);
        perror("Executing tail");
        exit(1);
    }
    if(debug) fprintf(stderr,"(parent process>closing the read end of the pipe...)\n");
    close(pipeDesc[0]);
    if(debug) fprintf(stderr,"(parent process>waiting for child processes to terminate...)\n");
    waitpid(child1pid,NULL,0);
    waitpid(child2pid,NULL,0);
    if(debug) fprintf(stderr,"(parent process>exiting...)\n");
    exit(0);
}
