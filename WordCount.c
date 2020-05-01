#include <stdio.h>
#include <string.h>
#include "mpi.h"
#include "stdlib.h"
#include "dirent.h"
#include <unistd.h>

#define row 1
#define cols 1

void deallocaArrayMultiDim(int**x, int rows){
  for (int i=0; i<rows; i++)
  {
    free(x[i]);
  }
  free(x);
}

char** creaArrayParole(){
    int i=0;
    int j=0;
    char ch;
    char separatore='\n';
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
    /* print all the files and directories within directory */
    int numfile=0;
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

                    while((ch = fgetc(in_file)) != EOF)
                    {   
                        if(ch==separatore)
                        {   
                            parole[i][j]='\n';
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
    */
    int k=0;
    int numParole=0;
   
    //matrice di parole
    char **parole;

    //inserisco nell'array di stringhe ogni parola
    parole=creaArrayParole();

    //stampo parole, solo per controllo, si può poi eliminare
    int num=0;
    while(parole[num]){
        printf("%s",parole[num]);
        num++;
        }
    
    //MPI_Finalize ();
    return 0;
}