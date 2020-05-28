#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>


void fseek_filesize(const char *filename){
    FILE *fp = NULL;
    long off;

    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("failed to fopen %s\n", filename);
        exit(EXIT_FAILURE);
    }

    if (fseek(fp, 0, SEEK_END) == -1)
    {
        printf("failed to fseek %s\n", filename);
        exit(EXIT_FAILURE);
    }

    off = ftell(fp);
    if (off == (long)-1)
    {
        printf("failed to ftell %s\n", filename);
        exit(EXIT_FAILURE);
    }

    printf("[*] fseek_filesize - file: %s, size: %ld\n", filename, off);

    if (fclose(fp) != 0)
    {
        printf("failed to fclose %s\n", filename);
        exit(EXIT_FAILURE);
    }
}

void fstat_filesize(const char *filename){
    int fd;
    struct stat statbuf;

    fd = open(filename, O_RDONLY, S_IRUSR | S_IRGRP);
    if (fd == -1)
    {
        printf("failed to open %s\n", filename);
        exit(EXIT_FAILURE);
    }

    if (fstat(fd, &statbuf) == -1)
    {
        printf("failed to fstat %s\n", filename);
        exit(EXIT_FAILURE);
    }

    printf("[*] fstat_filesize - file: %s, size: %ld\n", filename, statbuf.st_size);

    if (close(fd) == -1)
    {
        printf("failed to fclose %s\n", filename);
        exit(EXIT_FAILURE);
    }
}

void seek_filesize(const char *filename){
    int fd;
    off_t off;

    if (filename == NULL)
    {
        printf("invalid filename\n");
        exit(EXIT_FAILURE);
    }

    fd = open(filename, O_RDONLY, S_IRUSR | S_IRGRP);
    if (fd == -1)
    {
        printf("failed to open %s\n", filename);
        exit(EXIT_FAILURE);
    }

    off = lseek(fd, 0, SEEK_END);
    if (off == (off_t)-1)
    {
        printf("failed to lseek %s\n", filename);
        exit(EXIT_FAILURE);
    }

    printf("[*] seek_filesize - file: %s, size: %ld\n", filename, off);

    if (close(fd) == -1)
    {
        printf("failed to close %s\n", filename);
        exit(EXIT_FAILURE);
    }
}


typedef struct {
	int rank;
    signed long start;
    signed long end;
    char nomefile[10][16];
}SplitPerProcesso;

typedef struct {
	int rank;
    signed long start;
    signed long end;
    char nomefile[16];
}SplitPerProcesso2;

typedef struct{
    signed long size;
    char nomefile[16];
}SizeFile;

SplitPerProcesso2 BPP2(long * bytePerProcesso , SizeFile * bytePerFile, int p){
    int i=0; //processi
    int j=0; //struttura
    int k=0; //file
    int n=0; //num file in struttura
    long rimanenza=0;
    long tot = 0;
    SplitPerProcesso2 s[20];

    while(i<p){
        s[j].start=rimanenza;
        signed long differenza = bytePerProcesso[i]-bytePerFile[k].size;      
        if(differenza>0 && k<10){   //se la taglia del processo è abbastanza grande 
                                    //da prendere in input tutta la grandezza del file
            printf("1= per processo %d  ho taglia %ld",i,bytePerProcesso[i]);
            bytePerProcesso[i]=bytePerProcesso[i]-bytePerFile[k].size;
            s[j].end=bytePerFile[k].size+rimanenza;
            strcpy(s[j].nomefile,bytePerFile[k].nomefile);
            printf(", con file %s , parto da %ld, arrivo a %ld , rimangono al processo %ld\n",s[j].nomefile,s[j].start,s[j].end,bytePerProcesso[i]);
            rimanenza=0;
            j++;
            k++;

            

        }else{
            signed long diff2=bytePerFile[k].size-bytePerProcesso[i];
            if(diff2<0){//quando k = 10, ovvero sono all'ultimo file
                        //e la differenza è < di 0 (non dovrebbe accadere)
                        //quindi riempio per la taglia rimanente del file il processo
                s[j].end=bytePerFile[k].size;
                printf("qui");

            }else{  //nel caso in cui la size del processo rimanente è minore della size del file
                    //quindi vado a riempire per quanto rimane, dicendo il nome del file, 
                    //e passo al processo successivo, rimanendo però con quel file
                bytePerFile[k].size=bytePerFile[k].size-bytePerProcesso[i];
                rimanenza=bytePerProcesso[i]+1;
                s[j].end=bytePerProcesso[i];
                printf("2= per processo %d  ho taglia %ld",i,bytePerProcesso[i]);
                bytePerProcesso[i]=0;
            }
            strcpy(s[j].nomefile,bytePerFile[k].nomefile);
            printf(", con file %s , parto da %ld, arrivo a %ld , rimangono al processo %ld\n",s[j].nomefile,s[j].start,s[j].end,bytePerProcesso[i]);

            n=0;
            i++;
            j++;
            s[j].start=rimanenza;
        }
    }
    return *s;
}

SplitPerProcesso BPP(long * bytePerProcesso , SizeFile * bytePerFile, int p){
    int i=0; //struttura e processi
    int k=0; //file
    int n=0; //num file in struttura
    long tot = 0;
    SplitPerProcesso s[p];

    while(i<p){

        if(i==0){
            s[i].start=0;
        }

        signed long differenza = bytePerProcesso[i]-bytePerFile[k].size;      
        if(differenza>0 && k<10){

            bytePerProcesso[i]=bytePerProcesso[i]-bytePerFile[k].size;
            tot += bytePerFile[k].size;
            strcpy(s[i].nomefile[n],bytePerFile[k].nomefile);
            n++;
            k++;

        }else{
            signed long diff2=bytePerFile[k].size-bytePerProcesso[i];
            if(diff2<0){
                tot += bytePerFile[k].size;
                s[i].end=tot;
            }else{
                bytePerFile[k].size=bytePerFile[k].size-bytePerProcesso[i];
                tot+=bytePerProcesso[i];
                s[i].end=tot-1;
            }
            strcpy(s[i].nomefile[n],bytePerFile[k].nomefile);
            
            n=0;
            i++;
            s[i].start=tot;
        }
    }
    return *s;
}

void ripartizioneElementi( signed long * arrayAppoggio, signed long sizeFile, int p){

    int modulo=sizeFile%p;  
    //è probiabile che per solo per alcuni processi avrò più elementi   
    
    //controllo se il modulo mi da resto > 0
    if(modulo!=0){
        //se da > 0 allroa vado a inizializzare un array di appoggio nel quale
        //inserisco inizialmente quanti elementi devono ricevere ogni processo 
        for(int i=0;i<p;i++){
            arrayAppoggio[i]=sizeFile/p;
        }
        int temp=0;
        
        //faccio un while decremetnando ogni volta il valore di modulo,
        //in quanto aggiungo un elemento per volta a ogni processo, 
        //cosi da avere una distribuzione equa
        while(modulo!=0){
            //uso una variabile temp che mi serve solo per andare ogni volta
            //ad aggiungere un elemento per il relativo processo
            if(temp<p){
                arrayAppoggio[temp]=arrayAppoggio[temp]+1;
                temp++;
            }else{
                temp=0;
                arrayAppoggio[temp]=arrayAppoggio[temp]+1;
                temp++;
            }
            modulo--;
        }
        //free(temp);
    }else{
        //altrimenti riempio normalmente questo array di appoggio con i valori della divisione,
        //esendo uguali per tutti i processi
        for(int i=0;i<p;i++){
            arrayAppoggio[i]=sizeFile/p;
        }
    }
    //free(modulo);
}


double stat_filesize(const char *filename){
    struct stat statbuf;

    if (stat(filename, &statbuf) == -1)
    {
        printf("failed to stat %s\n", filename);
        exit(EXIT_FAILURE);
    }

    //printf("[*] stat_filesize - file: %s, size: %ld\n", filename, statbuf.st_size);

    return statbuf.st_size;
}

int main(int argc, const char *argv[]){

    DIR *dir;
    struct dirent *ent;
    signed long sizeTotFile=0;
    SizeFile sizePerFile[10]={0," "};
    
    int p=4;
    signed long bytePerProcesso[p];
    
    char cwd[PATH_MAX];
 
    //get path of current directory
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
       printf("Current working dir: %s\n", cwd);
    } else {
       perror("getcwd() error");
       
    }
    //add path folder 
    strcat(cwd,"/file/");

    if ((dir = opendir(cwd)) != NULL) {
        //untill the dir is empty, so for each file i take the word and put it all in the array
        //after this array will be splitted equally for each processor
        int i=0;
        while ((ent = readdir (dir)) != NULL) {
            if(strcmp(ent->d_name,".")!=0 && strcmp(ent->d_name,"..")!=0){
                FILE *in_file;
                char stringa[30]="file/";
                strcat(stringa,ent->d_name);
                //printf("%s\n",stringa);

                sizePerFile[i].size=stat_filesize(stringa);
                strcpy(sizePerFile[i].nomefile,stringa);
                sizeTotFile+=sizePerFile[i].size;
                printf("grandezza per file %s = %ld \n", stringa , sizePerFile[i].size);
                i++;
            }
            
        }
    }
    printf("size tot file %ld\n",sizeTotFile);

    ripartizioneElementi(bytePerProcesso,sizeTotFile,p);

    SplitPerProcesso2 s = BPP2(bytePerProcesso,sizePerFile,p);

    return 0;
}