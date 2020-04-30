#include <stdio.h>
#include <string.h>
#include "mpi.h"
#include "stdlib.h"

#define row 3
#define cols 1

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

    //crea nome file per poterli aprire
    for(int p=0;p<10;p++){
        char nomeFile[]="file";
        char numero=p+'0';
        strncat(nomeFile,&numero,1);
        printf("%s\n",nomeFile);
    }

    FILE *in_file  = fopen("file/provaIn.txt", "r"); // read only 
    //FILE *out_file = fopen("~/provaOut.txt", "w"); // write only 
    int i=0;
    int j=0;
    int k=0;
    char ch;
    char separatore='\n';

    //matrice di parole
    char **parole;
    parole=(char**)malloc(row*sizeof(char*));
    for(int alfa=0;alfa<row;alfa++)
        parole[alfa]=malloc(cols*sizeof(char));

    // test for files not existing. 
    if (in_file == NULL) 
    {   
        printf("Error! Could not open file\n"); 
        exit(-1); // must include stdlib.h 
    }

    //inserisco nell'array di stringhe ogni parola
    while((ch = fgetc(in_file)) != EOF)
    {   
        if(ch==separatore)
        {   
            parole[j][i]='\n';
            j++;    
            //se supero il numero di righe, quindi ho più parole di quante ne ho assegnate inizialmente
            //non essendo ancora finito il file, gli aggiugno un'altra riga per l'iterazione successiva
            //farlo tante volte mi fa perdere un po' di tempo ma non spreco memoria
            if(j>=row){
                parole=realloc(parole,sizeof(char*)*(j+1));
                parole[j]=malloc(cols*sizeof(char)); 
            }
            i=0;
        }else{
            parole[j][i]=ch;
            i++;
            //se supero il numero di colonne, quindi la lunghezza della parola, non essendo ancora 
            //la parola finita, allora aggiugno 1 altro spazio per l'iterazione successiva
            //farlo tante volte mi fa perdere un po' di tempo ma non spreco memoria
            if(i>=cols){
                parole[j]=realloc(parole[j],sizeof(char)*(i+1));   
            }   
        }   
    }    

    //stampo parole, solo per controllo, si può poi eliminare 
    for(int h=0;h<j;h++){
        k=0;
        while(parole[h][k]){
            printf("%c",parole[h][k]);
            k++;
        }
    }

    fclose(in_file);
    //fclose(out_file);    
    
    //MPI_Finalize ();
    return 0;
}