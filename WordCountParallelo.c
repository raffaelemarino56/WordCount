#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <dirent.h>
#include <mpi.h>

#define row 100
#define cols 1

typedef struct {
	char *parola;
	int frequenza;
}Word;

void deallocaArrayMultiDim(char**x, int rows){
  for (int i=0; i<rows; i++)
  {
    free(x[i]);
  }
  free(x);
}

int creaStruttura(char **parole,Word *words){

    int num=0;
    int contaStruttura=0;

    //per ogni parola presente nell'array dobbiamo contare la frequenza di questa
    while(parole[num]){
        //qui controllo se la parola che sto analizzando non sia vuota, questo perchè
        //se quando trovo una corrispondenza, io quella parola non devo più analizzarla, e quindi
        //quando la trovo le imposto il valore ""
        if(strcmp(parole[num],"")!=0){
            words[contaStruttura].frequenza=1;
            words[contaStruttura].parola=parole[num];
            //questo for mi consente di analizzare dalla parola immediatamente successiva a quella che ho
            //con il resto dell'array
            for(int a = num+1; a<row; a++){
                //se trova la corrispondenza allroa vado ad aumentare la frequenza di tale parola
                //ovviamente deve continuare a cercare nel caso in cui trova altre parole uguali
                if(strcmp(parole[a],parole[num])==0){
                    words[contaStruttura].frequenza=words[contaStruttura].frequenza+1;
                    //quando trova la parola, imposta nell'array tale parola a ""
                    //cosi il while principale, trova la parola "" non andrà a fare questo for
                    //risparmiando tempo. Come se in qualche modo mi segno che ho già analizzato questa parola
                    parole[a]="";
                }
            }
        contaStruttura++;
        }
        num++;
    }

    return num;
}

void creaCSV(char**parole){

    FILE *fpcsv;
    int num=0;
    int contaStruttura=0;

    Word words[row];
   
    fpcsv=fopen("occorrenze.csv","w+"); //apro in scrittura(se già esiste sovrascrive) file csv
    fprintf(fpcsv,"OCCORRENZA,PAROLA"); //prima riga file csv
    //per ogni parola presente nell'array dobbiamo contare la frequenza di questa
    while(parole[num]){
        //qui controllo se la parola che sto analizzando non sia vuota, questo perchè
        //se quando trovo una corrispondenza, io quella parola non devo più analizzarla, e quindi
        //quando la trovo le imposto il valore ""
        if(strcmp(parole[num],"")!=0){
            words[contaStruttura].frequenza=1;
            words[contaStruttura].parola=parole[num];
            //questo for mi consente di analizzare dalla parola immediatamente successiva a quella che ho
            //con il resto dell'array
            for(int a = num+1; a<row; a++){
                //se trova la corrispondenza allroa vado ad aumentare la frequenza di tale parola
                //ovviamente deve continuare a cercare nel caso in cui trova altre parole uguali
                if(strcmp(parole[a],parole[num])==0){
                    words[contaStruttura].frequenza=words[contaStruttura].frequenza+1;
                    //quando trova la parola, imposta nell'array tale parola a ""
                    //cosi il while principale, trova la parola "" non andrà a fare questo for
                    //risparmiando tempo. Come se in qualche modo mi segno che ho già analizzato questa parola
                    parole[a]="";
                }
            }
        fprintf(fpcsv,"\n%s,%d",words[contaStruttura].parola, words[contaStruttura].frequenza); //scrivo in file csv
        contaStruttura++;
        }
        num++;
    }
    fclose(fpcsv); //chiudo file csv

    //size_t numeroParole = sizeof(parole[0]);
    printf("tot parole contate %d\n",num);
}

char** creaArrayParole(){
    int i=0; //contatore per righe
    int j=0; //contatore per colonne
    char ch; 
    char separatore='\n';
    char terminatore='.';
    DIR *dir;
    struct dirent *ent;
    char cwd[PATH_MAX];

    //inizializzo l'array
    char **parole;
    parole=(char**)malloc(row*sizeof(char*));
    for(int alfa=0;alfa<row;alfa++)
        parole[alfa]=malloc(cols*sizeof(char));
   
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
                        if(ch==separatore)
                        {   
                            //parole[i][j]='\n';
                            printf("%s\n",parole[i]);
                            i++;
                            //se supero il numero di righe, quindi ho più parole di quante ne ho assegnate inizialmente
                            //non essendo ancora finito il file, gli aggiugno un'altra riga per l'iterazione successiva
                            //farlo tante volte mi fa perdere un po' di tempo ma non spreco memoria    
                            if(i>=row){
                                parole=realloc(parole,sizeof(char*)*(i+1));
                                parole[i]=malloc(cols*sizeof(char)); 
                            }
                            j=0;
                        }else{
                            //se supero il numero di colonne, quindi la lunghezza della parola, non essendo ancora 
                            //la parola finita, allora aggiugno 1 altro spazio per l'iterazione successiva
                            //farlo tante volte mi fa perdere un po' di tempo ma non spreco memoria
                            if(j>=cols){
                                parole[i]=realloc(parole[i],sizeof(char)*(j+1));   
                            }  
                            parole[i][j]=ch;
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
    return parole;
}

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

int main(int argc , char* argv[]){
	int myRank, p;
    int tag=0;
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    
    int arrayRipartizione[p];
    
    //struttura di grandezza  almeno quante sono le parole
    //poteri avere tutte parole diverse
    //Word words[row];
    
    //matrice di parole
    //char **parole;
 
    if(myRank==0){
        //quanti elementi dare a ogni processo  
        ripartizioneElementi(arrayRipartizione,p);

        //inserisco nell'array di stringhe ogni parola
        //MI DA PROBLEMI, CON PIU' PROCESSI METTE IMMONDIZIA ASSIEME ALLE PAROLE
        /*clock_t begin = clock();
        parole=creaArrayParole();
        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        printf("execution time = %lf\n",time_spent);
        */

        //int numStruttura=creaStruttura(parole,words);

        //invio ai processi la loro porzione di array
        /*for(int j=1; j<p; j++){
            int start=(j)*arrayRipartizione[j];
            MPI_Ssend(&parole[start], arrayRipartizione[j], MPI_CHAR, j, tag, MPI_COMM_WORLD);
            //MPI_Send(&words[start].parole, arrayRipartizione[j], MPI_CHAR, j, tag, MPI_COMM_WORLD);   
            //MPI_Send(&words[start].frequenza, arrayRipartizione[j], MPI_CHAR, j, tag, MPI_COMM_WORLD);           
        } */
    }

    //deallocaArrayMultiDim(parole,row);
  
    MPI_Finalize();
    return 0;
}