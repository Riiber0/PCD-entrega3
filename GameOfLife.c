#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "mpi.h"

#define PROX ((processId + 1) % noProcesses)
#define ANTE ((noProcesses + processId - 1) % noProcesses)
#define SIZE 2048
#define CHUNK_SIZE 2048/noProcesses + 2

int** tab_init(int chunk){

	int **ret = (int**)malloc(sizeof(int*) * chunk);

	for(int i = 0; i < chunk; i++){
		ret[i] = (int*)malloc(sizeof(int) * 2048);
		bzero(ret[i], sizeof(int) * 2048);
	}


	return ret;

}

void tab_finish(int **tab, int chunk){

	for(int i = 0; i < chunk; i++)
		free(tab[i]);

	free(tab);

}

void comunicacao(int **tab, int prox, int ante, int chunk, MPI_Status status){

	int *buf_1 = (int*)malloc(sizeof(int) * SIZE);

	MPI_Sendrecv(tab[chunk-2], SIZE, MPI_INTEGER, prox, 7765, 
				 buf_1, SIZE, MPI_INTEGER, ante, 7765, MPI_COMM_WORLD, &status);

	tab[0] = buf_1;
	int *buf_2 = (int*)malloc(sizeof(int) * SIZE);

	MPI_Sendrecv(tab[1], SIZE, MPI_INTEGER, ante, 7764, 
				 buf_2, SIZE, MPI_INTEGER, prox, 7764, MPI_COMM_WORLD, &status);

	tab[chunk-1] = buf_2;

	buf_1 = buf_2 = NULL;
	free(buf_2);
	free(buf_1);

}

int main(int argc, char** argv){

	int processId; /* rank dos processos */
	int noProcesses; /* Número de processos */
	int nameSize; /* Tamanho do nome */
	char computerName[MPI_MAX_PROCESSOR_NAME];
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &noProcesses);
	MPI_Comm_rank(MPI_COMM_WORLD, &processId);
	MPI_Get_processor_name(computerName, &nameSize);

	int **grid = tab_init(CHUNK_SIZE);

	if(!processId){
		//GLIDER
		int lin = 2, col = 0;
		grid[lin  ][col+1] = 1;
		grid[lin+1][col+2] = 1;
		grid[lin+2][col  ] = 1;
		grid[lin+2][col+1] = 1;
		grid[lin+2][col+2] = 1;
		 
		//R-pentomino
		lin =11; col = 30;
		grid[lin  ][col+1] = 1;
		grid[lin  ][col+2] = 1;
		grid[lin+1][col  ] = 1;
		grid[lin+1][col+1] = 1;
		grid[lin+2][col+1] = 1;

		grid[1][0] = 9;
	}

	MPI_Barrier(MPI_COMM_WORLD);
	int prox = PROX, ante = ANTE, chunk = CHUNK_SIZE;

	comunicacao(grid, prox, ante, chunk, status);

	if(processId == 1){
		printf("%d\n",grid[CHUNK_SIZE - 1][0]);
	}

	
	tab_finish(grid, CHUNK_SIZE);

	MPI_Finalize();
	return 0;

}
