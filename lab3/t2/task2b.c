#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    unsigned char* sig;
} virus;

typedef struct link link;
 
struct link {
    link *nextVirus;
    virus *vir;
};

void PrintHex(unsigned char *buffer,size_t length){
    for (size_t i = 0; i < length; i++)
        printf("%02X ",buffer[i]);
}

virus* readVirus(FILE* input){
    virus* vir = (virus*) malloc(sizeof(virus));
    char arr[18];
    int read;
    if ( (read = fread(arr,sizeof(char),18,input)) < 18 ){
        free(vir);
        return NULL;
    }
    vir->SigSize = arr[1]<<8 | arr[0]; //convert to little endian
    strcpy(vir->virusName,arr+2);
    vir->sig = (unsigned char *)malloc((vir->SigSize)*sizeof(unsigned char));
    if ( (read = fread(vir->sig,sizeof(unsigned char),vir->SigSize,input)) < vir->SigSize ){
        free(vir->sig);
        free(vir);
        return NULL;
    }
    return vir;
}

void printVirus(virus* virus, FILE* output){
    fprintf(output, "Virus name: %s\n",virus->virusName);
    fprintf(output,"Virus size: %d\n",virus->SigSize);
    fprintf(output,"signature:\n");
    PrintHex(virus->sig,virus->SigSize);
    fprintf(output,"\n\n");
}

void list_print(link *virus_list, FILE* output){
    link *virusLink = virus_list;
    while (virusLink != NULL){
        printVirus(virusLink->vir,output);
        virusLink = virusLink->nextVirus;
    }
}

link* list_append(link* virus_list, virus* data){
    link *newLink = (link*) malloc(sizeof(link));
    newLink->vir = data;
    newLink->nextVirus = NULL;
    if(virus_list == NULL) 
        return newLink;
    link *virusLink = virus_list;
    while (virusLink->nextVirus != NULL){
        virusLink = virusLink->nextVirus;
    }
    virusLink -> nextVirus = newLink;
    return virus_list;
}

void list_free(link *virus_list){
    link *virusLink = virus_list;
    while(virusLink != NULL){
        free(virusLink -> vir -> sig);
        free(virusLink -> vir);
        link *temp = virusLink;
        virusLink = virusLink->nextVirus;
        free(temp);
    }
}

void detect_virus(char *buffer, unsigned int size, link *virus_list, FILE* output){
    for (size_t i = 0; i < size; i++){
        for(link *virusLink = virus_list; virusLink != NULL; virusLink = virusLink->nextVirus){
            if ( memcmp(buffer+i,virusLink->vir->sig,virusLink->vir->SigSize) == 0){
                fprintf(output,"The starting byte is: %d\n", i+1);
                fprintf(output,"The name of the virus is: %s\n", virusLink->vir->virusName);
                fprintf(output,"The size of the signature: %d\n\n", virusLink->vir->SigSize);
            }
        }
    }
    
}

void kill_virus(char *fileName, int signitureOffset, int signitureSize){
    FILE *file = fopen(fileName,"rb+");
    fseek(file,signitureOffset-1,SEEK_SET);
    char arr[signitureSize];
    for (size_t i = 0; i < signitureSize ; i++){
        arr[i] = 0x90;
    }
    fwrite(arr, sizeof(char), signitureSize, file);
    fclose(file);
}

int main(int argc, char const *argv[]){ 
    link *linkedList = NULL;
    while(1){
        printf("1) Load signatures\n2) Print signatures\n3) Detect viruses\n4) Fix file\n");

        printf("Option: ");
        char optionS[3] = {'\0'};
        fgets(optionS,3,stdin);
        int option = atoi(optionS);
        if(option == 1){
            char fileName[100] = {'\0'};
            fgets(fileName,100,stdin);
            sscanf(fileName,"%[^'\n']",fileName);
            FILE *file = fopen(fileName, "rb");
            if (file == NULL){
                printf("NO SUCH FILE EXISTS!\n");
                continue;
            }
            
            char arr[4];
            fread(arr,sizeof(char),4,file);
            virus *readv = NULL;
            while ((readv = readVirus(file)) != NULL){
                linkedList = list_append(linkedList,readv);
            }
            fclose(file);
        }else if(option == 2){
            if(linkedList != NULL)
                list_print(linkedList,stdout);
        }else if(option == 3){
            FILE *file = fopen(argv[1], "rb");
            if (file == NULL){
                printf("NO SUCH FILE EXISTS!\n");
                continue;
            }
            char buffer[10000] = {'\0'};
            int read = fread(buffer,sizeof(char),10000,file);
            detect_virus(buffer, read, linkedList, stdout);
            fclose(file);
        }else if (option == 4){
            //get signiture offset
            printf("signiture offset: ");
            char signitureOffsetString[7] = {'\0'};
            fgets(signitureOffsetString,7,stdin);
            int signitureOffset = atoi(signitureOffsetString);

            //get signiture size
            printf("signiture size: ");
            char signitureSizeString[7] = {'\0'};
            fgets(signitureSizeString,7,stdin);
            int signitureSize = atoi(signitureSizeString);

            //get the file name as char*
            char filename[100] = {'\0'};
            strcpy(filename,argv[1]);

            //kill the virus
            kill_virus(filename,signitureOffset,signitureSize);

        }else{
            if(linkedList != NULL)
                list_free(linkedList);
            exit(1);
        }
    }

    return 0;  
}
