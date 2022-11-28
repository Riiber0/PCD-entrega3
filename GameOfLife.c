#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "mpi.h"

#define prox ((processId + 1) % noProcesses)

int** tab_init(int noProcesses){

	int **ret = (int**)malloc(sizeof(int*) * 2048/noProcesses + 2);

	for(int i = 0; i < 2048/noProcesses + 2; i++){
		ret[i] = (int*)malloc(sizeof(int) * 2048);
		bzero(ret[i], sizeof(int) * 2048);
	}


	return ret;

}

int main(int argc, char** argv){

	int processId; /* rank dos processos */
	int noProcesses; /* Número de processos */
	int nameSize; /* Tamanho do nome */
	char computerName[MPI_MAX_PROCESSOR_NAME];

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &noProcesses);
	MPI_Comm_rank(MPI_COMM_WORLD, &processId);
	MPI_Get_processor_name(computerName, &nameSize);

	int **grid = tab_init(noProcesses);

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
	}

	MPI_Barrier(MPI_COMM_WORLD);

	printf("%d prox-> %d\n", processId, ((processId + 1) % noProcesses));fflush(stdout);
	
	free(grid);
	return 0;

}
