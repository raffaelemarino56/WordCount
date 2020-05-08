#include <stdio.h>
#include <string.h>
#include "mpi.h"
#include "stdlib.h"
#include "dirent.h"
#include <unistd.h>
#include <time.h>
#define row 1010
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
                        if(ch==separatore || ch==terminatore)
                        {   
                            //parole[i][j]='\n';
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
                            parole[i][j]=ch;
                            j++;
                            //se supero il numero di colonne, quindi la lunghezza della parola, non essendo ancora 
                            //la parola finita, allora aggiugno 1 altro spazio per l'iterazione successiva
                            //farlo tante volte mi fa perdere un po' di tempo ma non spreco memoria
                            if(j>=cols){
                                parole[i]=realloc(parole[i],sizeof(char)*(j+1));   
                            }   
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

int main(int argc , char* argv[]){
    /*
    int my_rank;
    int data1 = 42;
    int data2 = 43;
    MPI_Status status;
    MPI_Request request;
    MPI_Init (&argc , &argv);
    MPI_Comm_rank(MPI_COMM_WORLD , &my_rank);
    
    double execution_time = 0.0;
    */
    
    int k=0;
    int numParole=0;
      
    //matrice di parole
    char **parole;

    //inserisco nell'array di stringhe ogni parola
    //execution_time -= MPI_Wtime();
    clock_t begin = clock();
    parole=creaArrayParole();
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("execution time = %lf\n",time_spent);
    //execution_time += MPI_Wtime();
    //printf("execution time: %lf\n",execution_time);

    creaCSV(parole);
  
    //MPI_Finalize ();
    return 0;
}