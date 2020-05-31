#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

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
        if(differenza>=0 && k<10){   //se la taglia del processo è abbastanza grande 
                                    //da prendere in input tutta la grandezza del file
            //printf("1= per processo %d  ho taglia %ld",i,bytePerProcesso[i]);
            bytePerProcesso[i]=bytePerProcesso[i]-bytePerFile[k].size;
            s[j].end=bytePerFile[k].size+rimanenza;
            strcpy(s[j].nomefile,bytePerFile[k].nomefile);
            //printf(", con file %s , parto da %ld, arrivo a %ld , rimangono al processo %ld\n",s[j].nomefile,s[j].start,s[j].end,bytePerProcesso[i]);
            rimanenza=0;
            j++;
            k++;
        }else{
            signed long diff2=bytePerFile[k].size-bytePerProcesso[i];
            if(diff2<0){//quando k = 10, ovvero sono all'ultimo file
                        //e la differenza è < di 0 (non dovrebbe accadere)
                        //quindi riempio per la taglia rimanente del file il processo
                s[j].end=bytePerFile[k].size;
                
            }else{  //nel caso in cui la size del processo rimanente è minore della size del file
                    //quindi vado a riempire per quanto rimane, dicendo il nome del file, 
                    //e passo al processo successivo, rimanendo però con quel file
                bytePerFile[k].size=bytePerFile[k].size-bytePerProcesso[i];
                rimanenza=bytePerProcesso[i]+1;
                s[j].end=bytePerProcesso[i];
                //printf("2= per processo %d  ho taglia %ld",i,bytePerProcesso[i]);
                bytePerProcesso[i]=0;
            }

            strcpy(s[j].nomefile,bytePerFile[k].nomefile);
        
            char file1[20]="/";
            char ch;
            char pf[PATH_MAX];
            //get path of current directory
            if (getcwd(pf, sizeof(pf)) != NULL) {
                //printf(" Current working dir: %s\n", pf);
            } else {
                perror("getcwd() error");
            }
            if(strcmp(s[j].nomefile,"")!=0){
                strcat(file1,s[j].nomefile);
                strcat(pf,file1);
                FILE *in_file;
                in_file = fopen(pf, "r");
                // test for files not existing. 
                if (in_file == NULL) 
                {   
                    printf("Error! Could not open file\n"); 
                    exit(-1);
                }

                fseek(in_file, s[j].end, SEEK_SET);
                ch = fgetc(in_file);
                do{
                    ch = fgetc(in_file);
                    s[j].end++;
                }while(ch!='\n' && isalpha(ch));
                s[j].end++;
                fclose(in_file);
            }

            //printf(", con file %s , parto da %ld, arrivo a %ld , rimangono al processo %ld\n",s[j].nomefile,s[j].start,s[j].end,bytePerProcesso[i]);
            //printf("\n");

            n=0;
            i++;
            j++;
            s[j].start=rimanenza;
        }
    }
    
}

void contaOccorrenzeCSV(Word *parole, int lunghezza){
    FILE *fpcsv;

    int num=0;
    fpcsv=fopen("occorrenze.csv","w+"); //apro in scrittura(se già esiste sovrascrive) file csv
    fprintf(fpcsv,"OCCORRENZA,PAROLA"); //prima riga file csv
    //per ogni parola presente nella struttura dobbiamo contare la frequenza di questa
    for(num = 0 ; num < lunghezza ; num++){
        //qui controllo se la parola che sto analizzando non sia vuota, questo perchè
        //se quando trovo una corrispondenza, io quella parola non devo più analizzarla, e quindi
        //quando la trovo le imposto il valore " "
        if(strcmp(parole[num].parola," ")!=0){
            //questo for mi consente di analizzare dalla parola immediatamente successiva a quella che ho
            //con il resto della struttura
            for(int a = num+1; a < lunghezza; a++){
                //se trova la corrispondenza allroa vado ad aumentare la frequenza di tale parola
                //ovviamente deve continuare a cercare nel caso in cui trova altre parole uguali
                if(strcmp(parole[a].parola,parole[num].parola)==0){
                        
                    parole[num].frequenza=parole[num].frequenza+parole[a].frequenza;
                    
                    //quando trova la parola, imposta nella struttura tale parola a " "
                    //cosi il for principale, trova la parola " " non andrà a fare questo for
                    //risparmiando tempo. Come se in qualche modo mi segno che ho già analizzato questa parola
                    strcpy(parole[a].parola," ");
                }
            }
            if(strcmp(parole[num].parola,"")!=0){
                fprintf(fpcsv,"\n%s,%d",parole[num].parola, parole[num].frequenza); //scrivo in file csv
            }
            
        }
    }
}

void contaOccorrenze(Word *parole, int lunghezza){
    int num=0;
    //per ogni parola presente nella struttura dobbiamo contare la frequenza di questa
    for(num = 0 ; num < lunghezza ; num++){
        //qui controllo se la parola che sto analizzando non sia vuota, questo perchè
        //se quando trovo una corrispondenza, io quella parola non devo più analizzarla, e quindi
        //quando la trovo le imposto il valore " "
        if(strcmp(parole[num].parola," ")!=0){
            //questo for mi consente di analizzare dalla parola immediatamente successiva a quella che ho
            //con il resto della struttura
            for(int a = num+1; a < lunghezza; a++){
                //se trova la corrispondenza allroa vado ad aumentare la frequenza di tale parola
                //ovviamente deve continuare a cercare nel caso in cui trova altre parole uguali
                if(strcmp(parole[a].parola,parole[num].parola)==0){
                    //essendo parole[a].frequenza = 1 è come se facessi +1
                    parole[num].frequenza=parole[num].frequenza+parole[a].frequenza; 
                    
                    //quando trova la parola, imposta nella struttura tale parola a " "
                    //cosi il for principale, trova la parola " " non andrà a fare questo for
                    //risparmiando tempo. Come se in qualche modo mi segno che ho già analizzato questa parola
                    strcpy(parole[a].parola," ");
                }
            }
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


int creaStrutturaParole(Word *parole, SplitPerProcesso2 * s, int count){
    int i=0; //contatore per righe
    int j=0; //contatore per colonne
    
    char ch; 
    char separatore='\n';
    char terminatore='.';
    
    for(int p = 0; p<count; p++){

        char cwd[PATH_MAX];
 
        //get path of current directory
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
        //printf("Current working dir: %s\n", cwd);
        } else {
        perror("getcwd() error");
        
        }
        //add path folder
        long conta = s[p].start;
        char file1[20]="/";
        strcat(file1,s[p].nomefile);
        strcat(cwd,file1);
        //printf("sono %d con nel file %s, parto da %ld, arrivo a %ld\n",s[p].rank,s[p].nomefile,conta,s[p].end);
        

        FILE *in_file;
        in_file = fopen(cwd, "r");
        // test for files not existing. 
        if (in_file == NULL) 
        {   
            printf("Error! Could not open file\n"); 
            exit(-1); // must include stdlib.h 
        }

        //se non parte dall'inizio del file
        fseek(in_file, conta, SEEK_SET);
        if(conta!=0){
            
            while(ch!='\n'){
                conta++;
                ch = fgetc(in_file);
            }
            
        }
        
        //finchè non arriva alla fine che doveva fare e non trova /n,
        //cosi se viene tagliato il file, e so che il processo successivo
        //che prenderà quel file non considera la prima parola
        //(se il file non parte da 0) verrà considerata nel processo corrente
        long finefile=s[p].end;
        fseek(in_file, conta, SEEK_SET);
        while(conta<finefile)
        {   
            ch = fgetc(in_file);
            if(ch==separatore){
                parole[i].frequenza=1;
                //printf("%s\n",parole[i].parola);
                i++;
                j=0;
            }else{
                if(isalpha(ch)){
                    parole[i].parola[j]=ch;
                    j++;
                }
            }   
            conta++;
        }

        //devo cercare di fare che se il file è taglaito devo prendermi fino a fine stringa, 
        //visto che il processo che si prende il file tagliato partirà da dopo quella stringa
        //printf("per %d , sono arrivato a %ld, dovevo arrivare a %ld, ho trovato %d parole\n",s[p].rank,conta,s[p].end,i);

        fclose(in_file);

    }
    return i;
}                



int main (int argc, char *argv[]){

    int numtasks, rank, source=0, tag=1;
    Word parole[row]={" ",0};
    MPI_Status stat;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    int count=3;
    MPI_Datatype wordtype,filePerProcType,oldtypes[count],oldtypes1[2];
    int blockcounts[count],blockcounts1[2];
    MPI_Aint offsets[count],offsets1[2],lb,extent;

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

    offsets1[0] = 0;
    oldtypes1[0] = MPI_CHAR;
    blockcounts1[0] = cols;
    
    MPI_Type_get_extent(MPI_CHAR, &lb, &extent); //lo uso per dire quanto spazio c'è fra il primo campo della struttura e il secondo
    offsets1[1] = offsetof(Word, parola); //appunto l'offset di quanti valori dal primo campo
    oldtypes1[1] = MPI_INT;
    blockcounts1[1] = 1;

    MPI_Type_create_struct(2, blockcounts1, offsets1, oldtypes1, &wordtype); //creo il tipo struttura
    MPI_Type_commit(&wordtype); 

    long bytePerProcesso[rank];
    long sizeTotFile=0;
    SizeFile sizePerFile[numfile];
    SplitPerProcesso2 s[splitprocesso];

    if(rank==0){
        printf("per processi #%d\n",numtasks);
        clock_t Kbegin = clock();
        //parte costante del programma essendo eseguita solo dal processo 0
        sizeTotFile=dimTotaleFile(sizePerFile);
        ripartizioneElementi(bytePerProcesso,sizeTotFile,numtasks);
        BPP2(s,bytePerProcesso,sizePerFile,numtasks);
        int k=0; //indice di dove mi trovo all'interno della struttura
        int startper0=0;
        int grandezzaperzero=0;
        
        //celle da passare a processo 1...
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
        //fine parte costante
        clock_t Kend = clock();
        double Ktime_spent = (double)(Kend - Kbegin) / CLOCKS_PER_SEC;
        printf("parte costante tempo impegato = %lf\n",Ktime_spent);

        //inizio conteggio parole, parte che varia a seconda dei processi
        clock_t Pbegin = clock();
        grandezzaperzero=creaStrutturaParole(parole,s,startper0);   
        contaOccorrenze(parole,grandezzaperzero);
        int grandezzaprocessi=0;
        int start2=grandezzaperzero;
        int quant=row-start2;
        
        for(int p = 1; p < numtasks; p++){

            MPI_Recv(&parole[start2], quant, wordtype, p, tag, MPI_COMM_WORLD, &stat);

            MPI_Get_count(&stat, wordtype, &grandezzaprocessi);

            start2=start2+grandezzaprocessi;
            quant=row-start2;
        }

        contaOccorrenzeCSV(parole,row);
        clock_t Pend = clock();
        double Ptime_spent = (double)(Pend - Pbegin) / CLOCKS_PER_SEC;
        printf("parte parallela tempo impegato = %lf\n",Ptime_spent);
        //fine
        double TimeTot=Ktime_spent+Ptime_spent;
        printf("tempo totale = %lf\n",TimeTot);
        printf("\n");
    }else{
        int count=0;
        int grandezzaStruttura=0;
        MPI_Recv(s, splitprocesso , filePerProcType, source, tag, MPI_COMM_WORLD, &stat);
        MPI_Get_count(&stat, filePerProcType, &count);
        
        grandezzaStruttura=creaStrutturaParole(parole,s,count);

        contaOccorrenze(parole,grandezzaStruttura);

        MPI_Send(parole, grandezzaStruttura, wordtype, 0, tag, MPI_COMM_WORLD);
        
    }

    MPI_Type_free(&wordtype);
    MPI_Type_free(&filePerProcType);
    MPI_Finalize();
    return 0;
}
