#include "util.h"

/* System calls */
#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_OPEN 5
#define SYS_CLOSE 6

/* Flags */
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define O_RDONLY 0
#define O_RDRW 2
#define O_CREAT 64

extern int system_call();

int main(int argc, char const *argv[]){
    /* updating program configuration/mode */
    int inputStream = STDIN;
    int outputStream = STDOUT;
    int i;
    for(i=1; i<argc; i++){
        if(strncmp(argv[i],"-i",2)==0) {
            inputStream = system_call(SYS_OPEN,argv[i]+2,O_RDONLY,0777);
            if(inputStream < 0 ) {
                char *str = "Error openning input file";
                system_call(SYS_WRITE,STDERR,str,strlen(str)); 
                return 1;
            }
        }
        if(strncmp(argv[i],"-o",2)==0) {
            outputStream = system_call(SYS_OPEN,argv[i]+2, O_RDRW | O_CREAT  ,0777);
            if(outputStream < 0 ) {
                char *str = "Error openning output file";
                system_call(SYS_WRITE,STDERR,str,strlen(str)); 
                return 1;
            }
        }
    }
    
    /* Running the encoder */
    char ch[1];
    int read = 1;
    while(read > 0){
        /* read from the user input */
        read = system_call(SYS_READ,inputStream,ch,1);
        /* encode the char */
        if((ch[0] <= 90) & (ch[0] >= 65))
            ch[0] += 32;
        /* write to output */
        if(read > 0)
            system_call(SYS_WRITE,outputStream,ch,1);
    
    }
    if(inputStream != STDIN) system_call(SYS_CLOSE,inputStream,0,0);
    if(outputStream != STDOUT) system_call(SYS_CLOSE,outputStream,0,0);
    
    return 0;
}