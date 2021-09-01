#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define  NAME_LEN  128
#define  BUF_SZ    10000
#define NUM_OF_FUNCTIONS 8
#define BUF_SIZE 40

typedef enum { false, true } bool;

typedef struct {
  bool debug_mode;
  char file_name[NAME_LEN];
  int unit_size;
  unsigned char mem_buf[BUF_SZ];
  size_t mem_count;
  /*
   .
   .
   Any additional fields you deem necessary
  */
} state;

typedef struct fun_desc {
  char *name;
  void (*fun)(state*);
} fun_desc;

void ToggleDebugMode(state* s);
void SetFileName(state* s);
void SetUnitSize(state* s);
void LoadIntoMemory(state* s);
void MemoryDisplay(state* s);
void SaveIntoFile(state* s);
void MemoryModify(state* s);
void Quit(state* s);
char* unit_to_format_hex(int unit);
char* unit_to_format_dec(int unit);
void print_units(int unit_size, int address, int count);
void print_buf(char *str, int address, int end, int unit_size, bool hex);

int main(int argc, char const *argv[]){
    state s;
    s.debug_mode=false;
    s.unit_size=1;
    s.mem_count=0;

    fun_desc funcsArray[] = {{"Toggle Debug Mode",ToggleDebugMode}, {"Set File Name",SetFileName}, {"Set Unit Size",SetUnitSize},
                             {"Load Into Memory",LoadIntoMemory},{"Memory Display",MemoryDisplay},{"Save Into File",SaveIntoFile},
                             {"Memory Modify",MemoryModify},{"Quit",Quit},{NULL,NULL}};
    while(true){
        fprintf(stderr,"Choose action:\n");
        for(int i = 0; funcsArray[i].name != NULL; i++){
            fprintf(stderr,"%d-%s\n", i, funcsArray[i].name);
        }
        fprintf(stderr,"> ");
        char input[3];
        fgets(input,3,stdin);
        int number;
        sscanf(input,"%d",&number);
        if(number < 0 || number > NUM_OF_FUNCTIONS-1){
            fprintf(stderr,"Please enter a valid number\n");
            continue;
        }
        (funcsArray[number].fun)(&s);
    }
}

void ToggleDebugMode(state* s){
    if(s->debug_mode){
        s->debug_mode = false;
        fprintf(stderr,"Debug flag now off\n\n");
    }else{
        s->debug_mode = true;
        fprintf(stderr,"Debug flag now on\n\n");
    }
}
void SetFileName(state* s){
    char fileName[NAME_LEN];
    fgets(fileName,NAME_LEN,stdin);
    sscanf(fileName,"%s",s->file_name);
    if(s->debug_mode) fprintf(stderr,"Debug: file name set to %s\n\n",s->file_name);
}
void SetUnitSize(state* s){
    char input[3];
    fgets(input,3,stdin);
    int number;
    sscanf(input,"%d",&number);
    if(number == 1 || number == 2 || number == 4){
        s->unit_size = number;
        if(s->debug_mode) fprintf(stderr,"Debug: set size to %d\n",s->unit_size);
    }else{
        fprintf(stderr,"Invalid size\n\n");
    }
    
}
void LoadIntoMemory(state* s){
    FILE *file = NULL;
    if(strcmp(s->file_name,"") == 0){
        fprintf(stderr,"File name is empty\n\n");
        return;
    }else if( (file = fopen(s->file_name,"r")) == NULL){
        fprintf(stderr,"Error opening file\n\n");
        return;
    }else{
        int location;
        int length;
        char input[BUF_SIZE];
        printf("Please enter <location> <length>\n");
        fgets(input,BUF_SIZE,stdin);
        sscanf(input,"%x %d",&location,&length);
        if(s->debug_mode) 
            fprintf(stderr,"File name: %s, Location: 0x%X, length: %d\n",s->file_name,location,length);
        fseek(file,location,SEEK_SET);
        fread(s->mem_buf,s->unit_size,length,file);
        fprintf(stderr,"Loaded %d units into memory\n\n",length);
        fclose(file);
    }
}
void MemoryDisplay(state* s){
    int address;
    int length;
    char input[BUF_SIZE];
    printf("Enter address and length\n");
    fgets(input,BUF_SIZE,stdin);
    sscanf(input,"%x %d",&address,&length);
    if(address == 0)
        address = (int) &s->mem_buf;
    print_units(s->unit_size,address,length);
}
void SaveIntoFile(state* s){
    FILE *file = NULL;
    if(strcmp(s->file_name,"") == 0){
        fprintf(stderr,"File name is empty\n\n");
        return;
    }else if( (file = fopen(s->file_name,"r+")) == NULL){
        fprintf(stderr,"Error opening file\n\n");
        return;
    }else{
        int sourceAddress;
        int targetLocation;
        int length;
        char input[BUF_SIZE];
        printf("Please enter <source-address> <target-location> <length>\n");
        fgets(input,BUF_SIZE,stdin);
        sscanf(input,"%x %d %d",&sourceAddress ,&targetLocation,&length);
        if(s->debug_mode) 
            fprintf(stderr,"File name: %s, Source address: 0x%x, Target location: %d, length: %d\n",s->file_name,sourceAddress,targetLocation,length);
        fseek(file,targetLocation,SEEK_SET);
        if(sourceAddress == 0)
            sourceAddress = (int) &s->mem_buf;
        char *buffer = (char *) sourceAddress;
        fwrite(buffer, s->unit_size, length, file);
        fprintf(stderr,"Wrote %d units into file\n\n",length);
        fclose(file);
    }
}
void MemoryModify(state* s){   
    int location;
    int val;
    printf("Please enter <location> <val>\n");
    char buffer[40];
    fgets(buffer,40,stdin);
    sscanf(buffer,"%x %x",&location,&val);
    if(s->debug_mode){
        fprintf(stderr, "location: %x, value: %x\n\n" ,location, val);
    }
    memcpy(s->mem_buf+location, &val, s->unit_size);
}
void Quit(state* s){
    if(s->debug_mode) fprintf(stderr,"quitting\n");
    exit(0);
}

char* unit_to_format_hex(int unit_size) {
    static char* formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
    return formats[unit_size-1];
}

char* unit_to_format_dec(int unit_size) {
    static char* formats[] = {"%#hhu\n", "%#hu\n", "No such unit", "%#u\n"};
    return formats[unit_size-1];
}

void print_units(int unit_size, int address, int count){
    int end = address + unit_size*count;
    print_buf("Hexadecimal\n", address,end,unit_size,true);//print hexadecimals
    print_buf("\nDecimal\n", address,end,unit_size,false); //print decimals
    fprintf(stderr,"\n");
}

void print_buf(char *str, int address, int end, int unit_size, bool hex){
    fprintf(stderr,"%s",str);
    fprintf(stderr,"=============\n");
    while (address < end) {
        //print in dec
        int var = *((int*)(address)); //var now is the byte in location address in the memory
        if(hex)
            fprintf(stderr, unit_to_format_hex(unit_size), var);
        else
            fprintf(stderr, unit_to_format_dec(unit_size), var);
        address += unit_size;
    }
}