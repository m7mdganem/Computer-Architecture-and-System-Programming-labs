#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "elf.h"

#define  NAME_LEN  128
#define  BUF_SZ    10000
#define NUM_OF_FUNCTIONS 6
#define BUF_SIZE 40

typedef enum { false, true } bool;

typedef struct {
  bool debug_mode;
  char file_name[NAME_LEN];
  int  Currentfd;
  int  len;
  void *map_start;
  Elf32_Ehdr *header;
} state;

typedef struct fun_desc {
  char *name;
  void (*fun)(state*);
} fun_desc;


void ToggleDebugMode(state* s);
void ExamineElfFile(state* s);
void PrintSectionNames(state* s);
void PrintSymbols(state* s);
void RelocationTables(state* s);
void Quit(state* s);

int main(int argc, char const *argv[]){
    state s;
    s.debug_mode=false;
    s.Currentfd = -1;
    s.map_start = NULL;
    s.len = -1;

    fun_desc funcsArray[] = {{"Toggle Debug Mode",ToggleDebugMode}, {"Examine ELF File",ExamineElfFile},
                             {"Print Section Names",PrintSectionNames}, {"Print Symbols",PrintSymbols},
                             {"Relocation Tables",RelocationTables}, {"Quit", Quit}, {NULL,NULL}};
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

void ExamineElfFile(state* s){
    char fileName[NAME_LEN];
    fprintf(stderr, "please enter file name: ");
    fgets(fileName,NAME_LEN,stdin);
    fprintf(stderr, "\n");
    sscanf(fileName,"%s\n",s->file_name);
    //unmap old mapping
    if(s->map_start != NULL){
        if(s->debug_mode) 
            fprintf(stderr,"unmapping old mapping\n");
        if(munmap(s->map_start, s->len) == -1){
            perror("unmapping old mapping");
            exit(EXIT_FAILURE);
        }
    }
    if(s->Currentfd != -1){
        if(s->debug_mode) 
            fprintf(stderr,"closing old file\n");
        if(close(s->Currentfd) != 0){
            perror("closing old file");
            exit(EXIT_FAILURE);
        }
    }
    if( (s->Currentfd = open(s->file_name,O_RDONLY)) == -1){
        perror("Unable to open file");
        return;
    }
    if(s->debug_mode) 
            fprintf(stderr,"oppened new file successfully\n");
    s->len = lseek(s->Currentfd,0,SEEK_END);
    lseek(s->Currentfd,0,SEEK_SET);
    if( (s->map_start = mmap(NULL,s->len,PROT_READ,MAP_PRIVATE,s->Currentfd,0)) == MAP_FAILED ){
        perror("mmap");
        close(s->Currentfd);
        s->Currentfd = -1;
        return;
    }
    if(s->debug_mode) 
            fprintf(stderr,"mapped new file successfully\n");
    
    s->header = (Elf32_Ehdr *) s->map_start;
    if(s->header->e_ident[EI_MAG0] != ELFMAG0 || s->header->e_ident[EI_MAG1] != ELFMAG1 
        || s->header->e_ident[EI_MAG2] != ELFMAG2 || s->header->e_ident[EI_MAG3] != ELFMAG3 
        || s->header->e_ident[EI_CLASS] != ELFCLASS32){
        if(s->debug_mode) 
            fprintf(stderr,"not ELF file, closing and unmapping\n");
        if(munmap(s->map_start, s->len) == -1){
            perror("unmapping");
            exit(EXIT_FAILURE);
        }
        s->map_start = NULL;
        if(close(s->Currentfd) != 0){
            perror("Closing");
            exit(EXIT_FAILURE);
        }
        s->Currentfd = -1;
        perror("Not ELF file");
        return;
    }

    fprintf(stderr, "ELF Header:\n");
    fprintf(stderr, "   %-30s %c%c%c\n", "Magic:", s->header->e_ident[EI_MAG1], s->header->e_ident[EI_MAG2], s->header->e_ident[EI_MAG3]);
    fprintf(stderr, "   %-30s %s\n", "Data", s->header->e_ident[EI_DATA] == ELFDATA2LSB ? "2's complement, little endian":
                                             s->header->e_ident[EI_DATA] == ELFDATA2MSB ? "2's complement, big endian":
                                             "Invalid data encoding");
    fprintf(stderr, "   %-30s 0x%x\n", "Entry point address:", s->header->e_entry);
    fprintf(stderr, "   %-30s %d %s\n", "Start of section headers:", s->header->e_shoff, "(bytes into file)");  
    fprintf(stderr, "   %-30s %d\n", "Number of section headers:", s->header->e_shnum);
    fprintf(stderr, "   %-30s %d %s\n", "Size of section headers:", s->header->e_shentsize, "(bytes)");
    fprintf(stderr, "   %-30s %d %s\n", "Start of program headers:", s->header->e_phoff, "(bytes into file)");
    fprintf(stderr, "   %-30s %d\n", "Number of program headers:", s->header->e_phnum);
    fprintf(stderr, "   %-30s %d %s\n", "Size of program headers:", s->header->e_phentsize, "(bytes)");
    fprintf(stderr, "\n");  
}

void PrintSectionNames(state* s){
    if(s->Currentfd == -1 || s->map_start == NULL){
        fprintf(stderr, "%s", "open file first");
        return;
    }
    Elf32_Shdr *sectionHeadersTable = (Elf32_Shdr *)(s->map_start + s->header->e_shoff);
    char *sectionHeadersStringTable = (char*)(s->map_start + sectionHeadersTable[s->header->e_shstrndx].sh_offset);
    if(s->debug_mode){
        fprintf(stderr, "section name string table index: %d, and its offset: %d\n", s->header->e_shstrndx, sectionHeadersTable[s->header->e_shstrndx].sh_offset);
    }
    fprintf(stderr,"%s %-20s %-15s %-15s %-15s %s\n", "[index]", "Name", "Addr", "Off", "Size", "Type");
    for(int i = 0; i < s->header->e_shnum; i++){
            fprintf(stderr, "[%-2d] %-20s    %08X        %06X          %06X          %s", i,sectionHeadersStringTable + sectionHeadersTable[i].sh_name, 
                    sectionHeadersTable[i].sh_addr, sectionHeadersTable[i].sh_offset, sectionHeadersTable[i].sh_size,
                    sectionHeadersTable[i].sh_type == SHT_NULL     ? "NULL":
                    sectionHeadersTable[i].sh_type == SHT_PROGBITS ? "PROGBITS":
                    sectionHeadersTable[i].sh_type == SHT_SYMTAB   ? "SYMTAB":
                    sectionHeadersTable[i].sh_type == SHT_STRTAB   ? "STRTAB":
                    sectionHeadersTable[i].sh_type == SHT_RELA     ? "RELA":
                    sectionHeadersTable[i].sh_type == SHT_HASH     ? "HASH":
                    sectionHeadersTable[i].sh_type == SHT_DYNAMIC  ? "DYNAMIC":
                    sectionHeadersTable[i].sh_type == SHT_NOTE     ? "NOTE":
                    sectionHeadersTable[i].sh_type == SHT_NOBITS   ? "NOBITS":
                    sectionHeadersTable[i].sh_type == SHT_REL      ? "REL":
                    sectionHeadersTable[i].sh_type == SHT_SHLIB    ? "SHLIB":
                    sectionHeadersTable[i].sh_type == SHT_DYNSYM   ? "DYNSYM":
                    sectionHeadersTable[i].sh_type == SHT_LOPROC   ? "LOPROC":
                    sectionHeadersTable[i].sh_type == SHT_HIPROC   ? "HIPROC":
                    sectionHeadersTable[i].sh_type == SHT_LOUSER   ? "LOUSER":
                    "HIUSER");
        fprintf(stderr, "\n");
    }
}

void PrintSymbols(state* s){
    bool printed = false;
    Elf32_Shdr *sectionHeadersTable = (Elf32_Shdr *)(s->map_start + s->header->e_shoff);
    char *sectionHeadersStringTable = (char*)(s->map_start + sectionHeadersTable[s->header->e_shstrndx].sh_offset);
    for(int i = 0; i < s->header->e_shnum; i++){
        if(sectionHeadersTable[i].sh_type == SHT_SYMTAB || sectionHeadersTable[i].sh_type == SHT_DYNSYM){
            Elf32_Sym *symbolTable = (Elf32_Sym*)(s->map_start + sectionHeadersTable[i].sh_offset);
            char *stringTable = (char *)(s->map_start + sectionHeadersTable[sectionHeadersTable[i].sh_link].sh_offset);
            int numOfSymEnts = sectionHeadersTable[i].sh_size / sizeof(Elf32_Sym);
            if(s->debug_mode){
                fprintf(stderr, "%s is of size %d and it have %d symbols\n", sectionHeadersStringTable+sectionHeadersTable[i].sh_name,
                        sectionHeadersTable[i].sh_size, numOfSymEnts);
            }
            fprintf(stderr,"%s  %-15s %-15s %-15s %-15s\n", "[index]", "value", "section index", "section name", "symbol name");
            for(int j = 0; j < numOfSymEnts; j++){
                if(symbolTable[j].st_shndx != SHN_LORESERVE && symbolTable[j].st_shndx != SHN_HIPROC  &&
                   symbolTable[j].st_shndx != SHN_ABS && symbolTable[j].st_shndx != SHN_COMMON && symbolTable[j].st_shndx != SHN_HIRESERVE) 
                fprintf(stderr,"[%-2d]     %08x        %-15d %-15s %-15s\n",j, symbolTable[j].st_value, symbolTable[j].st_shndx,
                        sectionHeadersStringTable + sectionHeadersTable[symbolTable[j].st_shndx].sh_name, 
                        stringTable + symbolTable[j].st_name);
                printed = true;
            }
            fprintf(stderr, "\n");
        }
    }
    if(printed == false){
        fprintf(stderr, "no symbols!");
        return;
    }
}


void RelocationTables(state* s){
    Elf32_Shdr *sectionHeadersTable = (Elf32_Shdr *)(s->map_start + s->header->e_shoff);
    char *sectionHeadersStringTable = (char*)(s->map_start + sectionHeadersTable[s->header->e_shstrndx].sh_offset);
    Elf32_Sym *symbolTable = NULL;
    char *stringTable = NULL ;
    int numOfSymEnts;
    for(int i = 0; i < s->header->e_shnum; i++){
        if(sectionHeadersTable[i].sh_type == SHT_DYNSYM){
            symbolTable = (Elf32_Sym *)(s->map_start + sectionHeadersTable[i].sh_offset);
            stringTable = (char *)(s->map_start + sectionHeadersTable[sectionHeadersTable[i].sh_link].sh_offset);
            numOfSymEnts = sectionHeadersTable[i].sh_size / sizeof(Elf32_Sym);
        }
    }
    if(symbolTable == NULL){
        fprintf(stderr, "no symbols!");
        return;
    }
    
    for(int i = 0; i < s->header->e_shnum; i++){
        if(sectionHeadersTable[i].sh_type == SHT_REL){
            Elf32_Rel *relocationTable = (Elf32_Rel *)(s->map_start + sectionHeadersTable[i].sh_offset);
            int numOfRelEnts = sectionHeadersTable[i].sh_size / sizeof(Elf32_Rel);
            if(s->debug_mode){
                fprintf(stderr, "Relocation section '%s' at offset 0x%X contains %d entries:\n", sectionHeadersStringTable+sectionHeadersTable[i].sh_name,
                        sectionHeadersTable[i].sh_offset, numOfRelEnts);
            }
            fprintf(stderr,"%-15s %-15s %-15s  %-13s %-15s\n", "Offset", "Info", "Type", "Sym.Value", "Sym. Name");
            for(int j = 0; j < numOfRelEnts; j++){
                int type = ELF32_R_TYPE(relocationTable[j].r_info);
                int symbolIndex = ELF32_R_SYM(relocationTable[j].r_info);
                fprintf(stderr, "%08X      %08X            %-10d      %08X   %-15s\n", relocationTable[j].r_offset, relocationTable[j].r_info,
                        type, symbolTable[symbolIndex].st_value, stringTable + symbolTable[symbolIndex].st_name);
            }
            fprintf(stderr,"\n");
        }
    }
}

void Quit(state* s){
    if(s->debug_mode) 
        fprintf(stderr,"quitting\n");
    if(s->map_start){
        if(munmap(s->map_start, s->len) == -1){
            perror("unmapping");
            exit(EXIT_FAILURE);
        }
        s->map_start = NULL;
        if(s->debug_mode) 
            fprintf(stderr,"unmapped file successfully\n");
    }
    if(s->Currentfd != -1){
        if (close(s->Currentfd) != 0){
            perror("Closing");
            exit(EXIT_FAILURE);
        }
        s->Currentfd = -1;
        if(s->debug_mode) 
            fprintf(stderr,"closed file successfully\n");
    }
    exit(EXIT_SUCCESS);
}