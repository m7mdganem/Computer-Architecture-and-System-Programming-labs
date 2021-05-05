#include <stdlib.h>
#include <stdio.h>
#include <string.h>
 
struct fun_desc {
  char *name;
  char (*fun)(char);
};

char censor(char c);
char encrypt(char c);
char decrypt(char c);
char cprt(char c);
char my_get(char c);
char* map(char *array, int array_length, char (*f) (char));
 
int main(int argc, char **argv){
    char *carray = (char*)malloc(5*sizeof(char));
    struct fun_desc menu[] = { { "Censor", censor }, { "Encrypt", encrypt }, 
                             { "Decrypt", decrypt }, { "Print string", cprt }, 
                             { "Get string", my_get }, { NULL, NULL } };

    while(1){
        int i = 0;
        for(i=0; menu[i].name != NULL; i++)
            printf("%d) %s\n", i, menu[i].name);

        printf("Option: ");
        char optionS[3] = {'\0'};
        fgets(optionS,3,stdin);
        int option = atoi(optionS);
        if(option>=0 && option<i){
            printf("Within bounds\n");
            char *mappedArray = map(carray,5,menu[option].fun);
            free(carray);
            carray = mappedArray;
        }else{
            printf("Not within bounds\n");
            free(carray);
            exit(1);
        }

        printf("Done.\n\n");
    }

    return 0;  
}

char censor(char c) {
  if(c == '!') return '.';
  else return c;
}

char encrypt(char c){
  if(c <= 0x7E & c >= 0x20) return c+3;
  return c;
}

char decrypt(char c){
  if(c <= 0x7E & c >= 0x20) return c-3;
  return c;
} 

char cprt(char c){
  if(c <= 0x7E & c >= 0x20) printf("%c\n",c);
  else{ fputc('.',stdout); fputc('\n',stdout);}
  return c;
}

char my_get(char c){
  return fgetc(stdin);
}


char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
  for (size_t i = 0; i < array_length; i++)
      mapped_array[i] = f(array[i]);

  return mapped_array;
}
