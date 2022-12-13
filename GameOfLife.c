#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "mpi.h"

#define PROX ((processId + 1) % noProcesses)
#define ANTE ((noProcesses + processId - 1) % noProcesses)
#define NUM_IT 2000
#define SIZE 2048
#define CHUNK_SIZE SIZE/noProcesses + 2

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

int** comunicacao(int **tab, int prox, int ante, int chunk, MPI_Status status){

	int *buf_1 = (int*)malloc(sizeof(int) * SIZE);
	bzero(buf_1, sizeof(int) * SIZE);
	int *buf_2 = (int*)malloc(sizeof(int) * SIZE);
	bzero(buf_2, sizeof(int) * SIZE);

	if(MPI_Sendrecv(tab[chunk-2], SIZE, MPI_INTEGER, prox, 7765, 
				 buf_1, SIZE, MPI_INTEGER, ante, 7765, MPI_COMM_WORLD, &status) == -1) perror("send 1");


	if(MPI_Sendrecv(tab[1], SIZE, MPI_INTEGER, ante, 7764, 
				 buf_2, SIZE, MPI_INTEGER, prox, 7764, MPI_COMM_WORLD, &status) == -1) perror("send 2");


	bzero(tab[0], sizeof(int) * SIZE);
	bzero(tab[chunk-1], sizeof(int) * SIZE);
	memcpy(tab[0], buf_1, sizeof(int) * SIZE);
	memcpy(tab[chunk-1], buf_2, sizeof(int) * SIZE);

	
	//buf_1 = buf_2 = NULL;
	free(buf_2);
	free(buf_1);
	

	return tab;

}

void printa(int** grid){
	int lin, col, sum = 0; 
	for(lin = 1; lin < 51; lin++){
		for(col = 0; col < 50; col++){
			printf("%d ",grid[lin][col]);
			if(grid[lin][col] == 1) sum++;
		}
		printf("\n");
	}
	printf("resultado dessa iteracao: %d\n\n",sum);
}

int conta_vizinho(int i, int j, int** grid){

	int cont = 0;
	if(grid[i - 1][(SIZE + j - 1) % SIZE] == 1) cont++;
	if(grid[i - 1][j ] == 1)cont++;
	if(grid[i - 1][(j + 1) % SIZE] == 1)cont++;
	if(grid[i ][(SIZE + j - 1) % SIZE] == 1)cont++;
	if(grid[i ][(j + 1) % SIZE] == 1)cont++;
	if(grid[i + 1][(SIZE + j - 1) % SIZE] == 1)cont++;
	if(grid[i + 1][j ] == 1)cont++;
	if(grid[i + 1][(j + 1) % SIZE] == 1)cont++;

	return cont;

}

int** iteracao(int **grid, int chunk, int prox, int ante, MPI_Status status, int id){

	int vizinhos, lins = chunk-1;
	int **ngrid = tab_init(chunk);
	int **swap;

	for(int it = 0; it < NUM_IT; it++){

		grid = comunicacao(grid, prox, ante, chunk, status);
		for(int i = 1; i < lins; i++){

			for(int j = 0; j < SIZE; j++){
				vizinhos = conta_vizinho(i, j, grid);
				
				if(grid[i][j] == 1 && (vizinhos == 2 || vizinhos == 3))ngrid[i][j] = 1;
				else if(grid[i][j] == 0 && vizinhos == 3)ngrid[i][j] = 1;
				else ngrid[i][j] = 0;
			}
		}

		swap = grid;
		grid = ngrid;
		ngrid = swap;

		if(id == 0 && it < 5) printa(grid);

	}

	return grid;

}

int conta(int **grid, int chunk){
	int lins = chunk - 1;
	int cont = 0;

	for(int i = 1; i < lins; i++){
		for(int j = 0; j < SIZE; j++){
			if(grid[i][j]) cont++;
		}
	}

	return cont;

}

int main(int argc, char** argv){

	int processId; /* rank dos processos */
	int noProcesses; /* Número de processos */
	int nameSize; /* Tamanho do nome */
	char computerName[MPI_MAX_PROCESSOR_NAME];
	MPI_Status status;

	struct timespec t1, t2;
	clock_gettime(1, &t1);

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &noProcesses);
	MPI_Comm_rank(MPI_COMM_WORLD, &processId);
	MPI_Get_processor_name(computerName, &nameSize);

	int **grid = tab_init(CHUNK_SIZE);

	if(processId == 0){
		//GLIDER
		int lin = 2, col = 1;
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

	grid = iteracao(grid, CHUNK_SIZE, PROX, ANTE, status, processId);

	int send_buf = conta(grid, CHUNK_SIZE);
	printf("%d - %d\n", processId, send_buf);
	int rcv_buf;
	MPI_Reduce(&send_buf, &rcv_buf, 1, MPI_INTEGER, MPI_SUM, 0, MPI_COMM_WORLD);
	if(processId == 0){

		printf("%d\n", rcv_buf);

	}

	
	tab_finish(grid, CHUNK_SIZE);

	MPI_Finalize();
	clock_gettime(1, &t2);
	int milsec_time = (t2.tv_sec - t1.tv_sec) * 1000;
	milsec_time += (t2.tv_nsec + t1.tv_nsec) / 1000000;

	printf("%d\n", milsec_time);

	return 0;

}
