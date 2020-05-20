#include "mpi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define row 16
#define cols 4

typedef struct {
	char parola[4];
	int frequenza;
}Word;

/*Sendind each colum to a processor*/
int main (int argc, char *argv[])
{
    int numtasks, rank, source=0, dest, tag=1, i;
    Word a[row];

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
    blockcounts[0] = 4;
    
    MPI_Type_get_extent(MPI_CHAR, &lb, &extent); //lo uso per dire quanto spazio c'è fra il primo campo della struttura e il secondo
    offsets[1] = 4 * extent; //appunto l'offset di quanti valori dal primo campo
    oldtypes[1] = MPI_INT;
    blockcounts[1] = 1;

    MPI_Type_create_struct(2, blockcounts, offsets, oldtypes, &wordtype); //creo il tipo struttura
    MPI_Type_commit(&wordtype); 

    if (numtasks == 4) {
    if (rank == 0) {

        strcpy(a[0].parola,"aaa");
        strcpy(a[1].parola,"bbb");
        strcpy(a[2].parola,"ccc");
        strcpy(a[3].parola,"ddd");
        strcpy(a[4].parola,"eee");
        strcpy(a[5].parola,"fff");
        strcpy(a[6].parola,"ggg");
        strcpy(a[7].parola,"hhh");
        strcpy(a[8].parola,"iii");
        strcpy(a[9].parola,"jjj");
        strcpy(a[10].parola,"kkk");
        strcpy(a[11].parola,"lll");
        strcpy(a[12].parola,"mmm");
        strcpy(a[13].parola,"nnn");
        strcpy(a[14].parola,"ooo");
        strcpy(a[15].parola,"ppp");
        for(int j=0;j<row;j++){
            a[j].frequenza=j;
        }

            for (i=0; i<numtasks; i++){
                start=i*numtasks;
                MPI_Send(&a[start], 4, wordtype, i, tag, MPI_COMM_WORLD);
            }
        }

        MPI_Recv(a, 4, wordtype, source, tag, MPI_COMM_WORLD, &stat);
        printf("rank= %d , PAROLE = b[0] = %s , b[1] = %s , b[2] = %s , b[3] = %s\n",rank,a[0].parola,a[1].parola,a[2].parola,a[3].parola);
        printf("rank= %d , FREQUENZE = b[0] = %d , b[1] = %d , b[2] = %d , b[3] = %d\n",rank,a[0].frequenza,a[1].frequenza,a[2].frequenza,a[3].frequenza);
        printf("\n");
    }else{
        printf("Must specify %d processors. Terminating.\n",4);
    }

    MPI_Type_free(&wordtype);
    MPI_Finalize();
    return 0;
}