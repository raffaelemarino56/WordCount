#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <mpi.h>


#define row 200000 //change this if the word for all file exceed (or is lowere) this value
                  //for now this is a static program, just beacuse i found some difficult with c 
                  //and the dinamic allocation of the memory
#define cols 16   //max lenght of the word

#define splitprocesso 20
#define numfile 10

typedef struct {
	char parola[cols];
	int frequenza;
}Word;

typedef struct {
	int rank;
    long start;
    long end;
    char nomefile[16];
}SplitPerProcesso2;

typedef struct{
    long size;
    char nomefile[16];
}SizeFile;

void BPP2(SplitPerProcesso2 * s , long * bytePerProcesso , SizeFile * bytePerFile, int p){
    int i=0; //processi
    int j=0; //struttura
    int k=0; //file
    int n=0; //num file in struttura
    long rimanenza=0;
    long tot = 0;
    
    while(i<p){
        s[j].rank=i;
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
            printf("\n");

            n=0;
            i++;
            j++;
            s[j].start=rimanenza;
        }
    }
    
}

void ripartizioneElementi( long * arrayAppoggio,  long sizeFile, int p){
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
       
    }else{
        //altrimenti riempio normalmente questo array di appoggio con i valori della divisione,
        //esendo uguali per tutti i processi
        for(int i=0;i<p;i++){
            arrayAppoggio[i]=sizeFile/p;
        }
    }
}


double stat_filesize(const char *filename){
    struct stat statbuf;

    if (stat(filename, &statbuf) == -1)
    {
        printf("failed to stat %s\n", filename);
        exit(EXIT_FAILURE);
    }

    return statbuf.st_size;
}

signed long dimTotaleFile(SizeFile * sizePerFile){

    DIR *dir;
    struct dirent *ent;
    signed long sizeTotFile=0;
    
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
                sizePerFile[i].size=stat_filesize(stringa);
                strcpy(sizePerFile[i].nomefile,stringa);
                sizeTotFile+=sizePerFile[i].size;
                i++;
            }
            
        }
    }
    return sizeTotFile;

}


void creaStrutturaParole(Word *parole, char * nomefile, long start, long end){
    int i=0; //contatore per righe
    int j=0; //contatore per colonne
    long conta=start;
    char ch; 
    char separatore='\n';
    char terminatore='.';
    DIR *dir;
    struct dirent *ent;
    char cwd[PATH_MAX];
 
    //get path of current directory
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
       printf("Current working dir: %s\n", cwd);
    } else {
       perror("getcwd() error");
       
    }
    //add path folder
    char file1[20]="/";
    strcat(file1,nomefile);
    strcat(cwd,file1);
    printf("sono nel file %s, parto da %ld, arrivo a %ld\n",nomefile,conta,end);
    

    FILE *in_file;
    in_file = fopen(cwd, "r");
    fseek(in_file, conta, SEEK_SET);
    if(conta!=0){
        while(ch!='\n'){
             conta++;
             ch = fgetc(in_file);
        }
        
    }
    
    // test for files not existing. 
    if (in_file == NULL) 
    {   
        printf("Error! Could not open file\n"); 
        exit(-1); // must include stdlib.h 
    }
    
    while(conta<end)
    {   
        ch = fgetc(in_file);
        if(ch==separatore){
            parole[i].frequenza=1;
            i++;
            j=0;
        }else{
            parole[i].parola[j]=ch;
            j++;
        }   
        conta++;
    }

    //devo cercare di fare che se il file è taglaito devo prendermi fino a fine stringa, 
    //visto che il processo che si prende il file tagliato partirà da dopo quella stringa
    printf("sono arrivato a %ld, dovevo arrivare a %ld\n",conta,end);

    fclose(in_file);
}                



int main (int argc, char *argv[]){

    int numtasks, rank, source=0, tag=1;
    int y=5, z=10;
    MPI_Status stat;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    int count=3;
    MPI_Datatype filePerProcType,oldtypes[count];
    int blockcounts[count];
    MPI_Aint offsets[count],lb,extent;

    offsets[0] = offsetof(SplitPerProcesso2, rank);
    oldtypes[0] = MPI_INT;
    blockcounts[0] = 1;
    MPI_Type_get_extent(MPI_INT,&lb,&extent);

    offsets[1] = offsetof(SplitPerProcesso2, start);
    oldtypes[1] = MPI_LONG;
    blockcounts[1] = 2;
    MPI_Type_get_extent(MPI_LONG,&lb,&extent);

    offsets[2] = offsetof(SplitPerProcesso2, nomefile);
    oldtypes[2] = MPI_CHAR;
    blockcounts[2] = 16;
    MPI_Type_get_extent(MPI_CHAR,&lb,&extent);

    MPI_Type_create_struct(count, blockcounts, offsets, oldtypes, &filePerProcType); //creo il tipo struttura
    MPI_Type_commit(&filePerProcType); 

    long bytePerProcesso[rank];
    long sizeTotFile=0;
    SizeFile sizePerFile[numfile];
    SplitPerProcesso2 s[splitprocesso];

    sizeTotFile=dimTotaleFile(sizePerFile);

    if(rank==0){
        
        ripartizioneElementi(bytePerProcesso,sizeTotFile,numtasks);
        BPP2(s,bytePerProcesso,sizePerFile,numtasks);
        int k=0; //indice di dove mi trovo all'interno della struttura
        int startper0=0;
        while(s[k].rank==0){
                k++;
                startper0++;
            }
            
        int q=startper0; //da dove devo partire
        for (int i=1; i<numtasks; i++){
            int j=0; //quanti elementi
            while(s[k].rank==i){
                k++;
                j++;
            }
            MPI_Send(&s[q], j , filePerProcType, i, tag, MPI_COMM_WORLD);
            q=k;
        }    

        for(int i=0; i<startper0; i++){
            
        }     
        //calcolo occorrenze
        //recive

    }else{
        MPI_Recv(s, splitprocesso , filePerProcType, source, tag, MPI_COMM_WORLD, &stat);
        int count;
        MPI_Get_count(&stat, filePerProcType, &count);
        Word parole[row]={" ",0}; //per inizializzare elementi struttua

        for(int i=0; i<count; i++){
            creaStrutturaParole(parole,s[i].nomefile,s[i].start,s[i].end);
        }
        //calcolo occorrenze
        //send

    }

    MPI_Type_free(&filePerProcType);
    MPI_Finalize();
    return 0;
}
