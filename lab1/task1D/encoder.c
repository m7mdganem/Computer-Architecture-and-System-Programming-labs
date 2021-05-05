#include <stdio.h>
#include <string.h>
typedef enum {false=0, true=1} bool;

int getEchar(const char c){
    if(c >= 48 & c <= 57) return c-'0';
    else if(c >= 65 & c <= 70) return c-55;
    return -1;
}

int main(int argc, char const *argv[]){
    
    //updating program configuration
    bool debugMode = false;
    bool encryptMode = false;
    bool increase = false; //indicating if the encryption is with + or -
    int eChar = 0; //indicating the encryption key
    FILE *inputStream = stdin;
    for(int i=1; i<argc; i++){
        if(strncmp(argv[i],"-D",2)==0) debugMode = true;
        else if (strncmp(argv[i],"+e",2)==0){
            encryptMode = true; 
            increase = true; 
            eChar = getEchar(argv[i][2]);
            if(eChar == -1){//if the given key is outside the range print error message and exit
                fprintf(stderr,"The encryption key shold be btween 0-F\n");
                return 1;
            }
        }
        else if (strncmp(argv[i],"-e",2)==0){
            encryptMode = true; 
            eChar = getEchar(argv[i][2]);
            if(eChar == -1){//if the given key is outside the range print error message and exit
                fprintf(stderr,"The encryption key shold be btween 0-F\n");
                return 1;
            }
        }
        else if (strncmp(argv[i],"-i",2)==0){
            inputStream = fopen(argv[i]+2,"r");
            if(inputStream == NULL){fprintf(stderr,"Error reading input file"); return 1;}
        }
        
    }
	if(debugMode) fprintf(stderr,"-D\n");

    //Run the encoder
    int ch;
    int counter = 0;
    while( (ch = fgetc(inputStream)) != EOF){
        //if in debugging mode print the value of the char before midifying
        if(debugMode & ch != '\n') fprintf(stderr,"%d    ",ch);
        
        //encode to lower case or encrypt
        if(!encryptMode){
            if (ch <= 90 & ch >= 65){ch += 32; counter++;}
        }else if(ch != '\n'){
            if(increase){ ch += eChar; counter++;}//in case of +e
            else{ch -= eChar; counter++;}//in case of -e
        }

        //if in debugging mode print the value of the char after midifying
        if(debugMode & ch != '\n') fprintf(stderr,"%d\n",ch);

        //if in debugging mode and finished a line : print num of modified chars and reset counter
        if(debugMode & ch == '\n'){
            fprintf(stderr,"the number of letters: %d\n",counter);
            counter = 0;
        }

        fputc(ch,stdout); //print the modified char
    }

    if(debugMode & ch == EOF & counter > 0){
            fprintf(stderr,"the number of letters: %d\n",counter);
            counter = 0;
            fputc('\n',stdout);
    }

    return 0;
}
