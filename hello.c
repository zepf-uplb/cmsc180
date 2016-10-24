#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <stdlib.h>

int **createMatrix(int n);
int printMatrix(int **M, int n);
int ***divideMatrix(int **M, int t, int n);
int freeMatrix(int **M, int n);

int main(int argc, char** argv)
{
    int myrank, nprocs, **M, n;

    n = 8;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    srand(time(NULL));


    //create matrix
    M = createMatrix(n);
    printMatrix(M, n);

    //divide to submatrix


    //start time

    freeMatrix(M, n);

    printf("Hello from processor %d of %d\n", myrank, nprocs);

    MPI_Finalize();

    return 0;
}

int **createMatrix(int n)
{
	int **M, i, j;

	M = (int**)malloc(sizeof(int*)*n);

	for(i = 0; i < n; i++){
		M[i] = (int*)malloc(sizeof(int)*n);
		for(j = 0; j < n; j++){			
			M[i][j] = (rand() % 10)+1;
		}
	}	

	return M;
}

int printMatrix(int **M, int n)
{
	int i, j;

	for(i = 0; i < n; i++){
		for(j = 0; j < n; j++){
			printf("%d ", M[i][j]);
		}
		printf("\n");
	}

	return 1;
}

int ***divideMatrix(int **M, int t, int n)
{
	int ***m, i, j, k, divs;

	m = (int***)malloc(sizeof(int**)*t);

	for(i = 0; i < t; i++){
		m[i] = (int**)malloc(sizeof(int*)*n);
		for(j = 0; j < n; j++){
			m[i][j] = (int*)malloc(sizeof(int)*(n/t));
		}
	}

	divs = n/t;

	if(n > divs*t){
		divs++;
	}

	for(k = 0; k < t; k++){
		for(i = 0; i < n; i++){
			for(j = 0; j < divs; j++){
				m[k][i][j] = M[i][j+(k*(divs))];
			}
		}
	}

	return m;
}

int freeMatrix(int **M, int n)
{
	int i;

	for(i = 0; i < n; i++){
		free(M[i]);
	}

	free(M);

	return 1;
}