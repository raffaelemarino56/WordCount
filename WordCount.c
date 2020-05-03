#include <stdio.h>
#include <string.h>
#include "mpi.h"
#include "stdlib.h"
#include "dirent.h"
#include <unistd.h>
#include <time.h>
#define row 1000
#define cols 1

typedef struct {
	char *parola;
	int frequenza;
}Word;

void deallocaArrayMultiDim(int**x, int rows){
  for (int i=0; i<rows; i++)
  {
    free(x[i]);
  }
  free(x);
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

                    do
                    {   
                        if(ch==separatore || ch==terminatore)
                        {   
                            parole[i][j]='\n';
                            //printf("parola = %s",parole[i]);
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
                    }while((ch = fgetc(in_file)) != terminatore);
                
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

    /*stampo parole, solo per controllo, si può poi eliminare*/
    int num=0;
    int contaStruttura=0;

    Word words[row];
    
    //words = malloc(row*sizeof(Word));

    while(parole[num]){
        if(strcmp(parole[num],"")!=0){
            words[contaStruttura].frequenza=1;
            words[contaStruttura].parola=parole[num];

            for(int a = num+1; a<num; a++){
                if(strcmp(parole[a],parole[num])==0){
                    printf("%s\n",parole[a]);
                    words[contaStruttura].frequenza=words[contaStruttura].frequenza=+1;
                    parole[a]="";
                }

                
            }
        contaStruttura++;
        }
        //printf("%d, %s \n",words[num].frequenza,words[num].parola);
        num++;
    }

    /*for(int q=0; q<contaStruttura;q++){
        printf("%d, %s \n",words[num].frequenza,words[num].parola);
    }*/
    //size_t numeroParole = sizeof(parole[0]);
    printf("tot parole contate %d\n",num);
    
    //MPI_Finalize ();
    return 0;
}