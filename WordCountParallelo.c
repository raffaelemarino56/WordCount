#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <dirent.h>
#include <mpi.h>

#define row 100
#define cols 16

typedef struct {
	char parola[cols];
	int frequenza;
}Word;


void ripartizioneElementi( int * arrayAppoggio, int p){

    //nella funzione di MPI_Recv penso dovrò usare MPI_Get_count(&status,MPI_INT,&count); 
    //dove count è dichiarata nel main come status

    int modulo=row%p;  
    //verrà usato nelle MPI_Send, faccio un for per ogni elemento
    //in quanto è probiabile che per solo per alcuni processi ho più elementi   
    
    //controllo se il modulo mi da resto > 0
    if(modulo!=0){
        //se da > 0 allroa vado a inizializzare un array di appoggio nel quale
        //inserisco inizialmente quanti elementi devono ricevere ogni processo 
        for(int i=0;i<p;i++){
            arrayAppoggio[i]=row/p;
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
            arrayAppoggio[i]=row/p;
        }
    }
    //free(modulo);
}

void creaStrutturaParole(Word *parole){
    int i=0; //contatore per righe
    int j=0; //contatore per colonne
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
    strcat(cwd,"/file/");

    if ((dir = opendir(cwd)) != NULL) {
        //untill the dir is empty, so for each file i take the word and put it all in the array
        //after this array will be splitted equally for each processor
        while ((ent = readdir (dir)) != NULL) {
            if(strcmp(ent->d_name,".")!=0 && strcmp(ent->d_name,"..")!=0){
                FILE *in_file;
                char stringa[30]="file/";
                strcat(stringa,ent->d_name);
                printf("%s\n",stringa);
                in_file = fopen(stringa, "r");
                    // test for files not existing. 
                    if (in_file == NULL) 
                    {   
                        printf("Error! Could not open file\n"); 
                        exit(-1); // must include stdlib.h 
                    }
                    
                    while((ch = fgetc(in_file)) != terminatore)
                    {   
                        if(ch==separatore){
                            parole[i].frequenza=1;
                            i++;
                            j=0;
                        }else{
                            parole[i].parola[j]=ch;
                            j++;
                        }   
                    }
                
                fclose(in_file);
            }                
        }
        closedir (dir);
    } else {
        /* could not open directory */
        printf("Error! Could not open directory\n"); 
        exit(-1); // must include stdlib.h 
    }
}

/*Sendind each colum to a processor*/
int main (int argc, char *argv[])
{
    int numtasks, rank, source=0, dest, tag=1, i;
    Word parole[row]={" ",0}; //per inizializzare elementi struttua
    Word paroleProcessi[row];
    
    int start;
    MPI_Status stat;

    MPI_Datatype wordtype,oldtypes[2]; //il nuovo tipo struttura e i due vecchi tipo usati nella struttura
    int blockcounts[2];
    MPI_Aint offsets[2], lb, extent;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    // il tipo MPI_CHAR usato per la parola
    offsets[0] = 0;
    oldtypes[0] = MPI_CHAR;
    blockcounts[0] = cols;
    
    MPI_Type_get_extent(MPI_CHAR, &lb, &extent); //lo uso per dire quanto spazio c'è fra il primo campo della struttura e il secondo
    offsets[1] = cols * extent; //appunto l'offset di quanti valori dal primo campo
    oldtypes[1] = MPI_INT;
    blockcounts[1] = 1;

    MPI_Type_create_struct(2, blockcounts, offsets, oldtypes, &wordtype); //creo il tipo struttura
    MPI_Type_commit(&wordtype); 

    int arrayAppoggio[numtasks];
    ripartizioneElementi(arrayAppoggio,numtasks);
    
    if (rank == 0) {
        //riempi struttura
        creaStrutturaParole(parole);

        for (i=1; i<numtasks; i++){
            //da dove deve partire, se il processo precedente fa 50elememti
            //il processo successivo dovà partire da quella posizione mettiamo caso che sono al processo 2 con 49 elementi, 
            //ed il processo 1 ha fatto già 50 elementi quindi il processo 3 partirà da posizione 99
            start=start+arrayAppoggio[i-1];
            MPI_Send(&parole[start], arrayAppoggio[i] , wordtype, i, tag, MPI_COMM_WORLD);
        }
        
        int num=0;

        //per ogni parola presente nell'array dobbiamo contare la frequenza di questa
        for(num = 0 ; num < arrayAppoggio[rank] ; num++){
            //qui controllo se la parola che sto analizzando non sia vuota, questo perchè
            //se quando trovo una corrispondenza, io quella parola non devo più analizzarla, e quindi
            //quando la trovo le imposto il valore ""
            if(strcmp(parole[num].parola,"")!=0){
                //questo for mi consente di analizzare dalla parola immediatamente successiva a quella che ho
                //con il resto dell'array
                for(int a = num+1; a<arrayAppoggio[rank]; a++){
                    //se trova la corrispondenza allroa vado ad aumentare la frequenza di tale parola
                    //ovviamente deve continuare a cercare nel caso in cui trova altre parole uguali
                    if(strcmp(parole[a].parola,parole[num].parola)==0){
                        
                        parole[num].frequenza=parole[num].frequenza+1;
                        
                        //quando trova la parola, imposta nell'array tale parola a ""
                        //cosi il while principale, trova la parola "" non andrà a fare questo for
                        //risparmiando tempo. Come se in qualche modo mi segno che ho già analizzato questa parola
                        strcpy(parole[a].parola," ");
                    }
                }
            }
            if(strcmp(parole[num].parola," ")!=0){
                printf("al rank %d , iterazione %d ,trovo la parola %s , con frequenza %d\n",rank,num,parole[num].parola,parole[num].frequenza);
            }            
        }

        for(int p = 1; p < numtasks; p++){
            MPI_Recv(paroleProcessi, arrayAppoggio[p] , wordtype, p, tag, MPI_COMM_WORLD, &stat);

        }
        
    }else{

        MPI_Recv(paroleProcessi, arrayAppoggio[rank] , wordtype, source, tag, MPI_COMM_WORLD, &stat);
        
        int num=0;
        
        //per ogni parola presente nell'array dobbiamo contare la frequenza di questa
        for(num = 0 ; num < arrayAppoggio[rank] ; num++){
            //qui controllo se la parola che sto analizzando non sia vuota, questo perchè
            //se quando trovo una corrispondenza, io quella parola non devo più analizzarla, e quindi
            //quando la trovo le imposto il valore ""
            if(strcmp(paroleProcessi[num].parola," ")!=0){
                //questo for mi consente di analizzare dalla parola immediatamente successiva a quella che ho
                //con il resto dell'array
                for(int a = num+1 ; a < arrayAppoggio[rank] ; a++){
                    //se trova la corrispondenza allroa vado ad aumentare la frequenza di tale parola
                    //ovviamente deve continuare a cercare nel caso in cui trova altre parole uguali
                    if(strcmp(paroleProcessi[a].parola,paroleProcessi[num].parola)==0){
                        
                        paroleProcessi[num].frequenza=paroleProcessi[num].frequenza+1;
                        
                        //quando trova la parola, imposta nell'array tale parola a ""
                        //cosi il while principale, trova la parola "" non andrà a fare questo for
                        //risparmiando tempo. Come se in qualche modo mi segno che ho già analizzato questa parola
                        strcpy(paroleProcessi[a].parola," ");
                    }
                }
            }
            if(strcmp(paroleProcessi[num].parola," ")!=0){
                printf("al rank %d , iterazione %d ,trovo la parola %s , con frequenza %d\n",rank,num,paroleProcessi[num].parola,paroleProcessi[num].frequenza);
            }
        }


        MPI_Send(&paroleProcessi[start], arrayAppoggio[rank] , wordtype, 0, tag, MPI_COMM_WORLD);
    }

    MPI_Type_free(&wordtype);
    MPI_Finalize();
    return 0;
}