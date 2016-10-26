#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>

int **createMatrix(int n);
int printMatrix(int **M, int n);
int ***divideMatrix(int **M, int t, int n);
int printSubmatrix(int ***m, int t, int n);
int printArray(int *A, int n);
int freeMatrix(int **M, int n);
int freeSubMatrix(int ***m, int t, int n);
int *matrixToArray(int **M, int n);
int *computeColumnSums(int *a, int n, int t);

int main(int argc, char** argv)
{
    int myrank, nprocs, **M, ***m, *A, *a, n, t, i, j, divs, *sums, *SUMS;

    n = 8, t = 4;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    srand(time(NULL));


    //create matrix
    if(myrank == 0){
    	M = createMatrix(n);  

    	printf("Matrix:\n");
    	printMatrix(M, n);
    
    	A = matrixToArray(M, n);

    	//printf("Array:\n");
    	//printArray(A, n);

    	freeMatrix(M, n);
    }
	     
    divs = n/t;

	if(n > divs*t){
		divs++;
	}

    a = (int*)malloc(sizeof(int)*(n*divs));
    assert(a != NULL);

    MPI_Scatter(A, (n*divs), MPI_INT, a, (n*divs), MPI_INT, 0, MPI_COMM_WORLD);

    /*printf("I am processor %d.\n", myrank);
    for(i = 0; i < (n*divs); i++){
    	printf("%d ", a[i]);
    }
    printf("\n");*/
    sums = computeColumnSums(a, n, t);

    printf("I am processor %d. :", myrank);
    for(i = 0; i < divs; i++){
    	printf("%d ", sums[i]);
    }	
    printf("\n");

    SUMS = (int*)malloc(sizeof(int)*n);
    assert(SUMS != NULL);

    MPI_Allgather(sums, divs, MPI_INT, SUMS, divs, MPI_INT, MPI_COMM_WORLD);

    if(myrank == 0){
    	printf("\n");
    	for(i = 0; i < n; i++){
	    	printf("%d ", SUMS[i]);
	    }
	    printf("\n");
	    //free(A);
    }
	    

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
			M[i][j] = (rand() % 9)+1;
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

int printSubmatrix(int ***m, int t, int n)
{
	int i, j, k, divs;

	divs = n/t;

	if(n > divs*t){
		divs++;
	}

	for(i = 0; i < t; i++){
		for(j = 0; j < n; j++){
			for(k = 0; k < divs; k++){
				printf("%d ", m[i][j][k]);
			}
			printf("\n");
		}
		printf("\n");
	}

	return 1;
}

int printArray(int *A, int n)
{
	int i;
	for(i = 0; i < n*n; i++){
		printf("%d ", A[i]);
	}
	printf("\n");

	return 1;
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

int freeSubMatrix(int ***m, int t, int n)
{
	int i, j;

	for(i = 0; i < t; i++){
		for(j = 0; j < n; j++){
			free(m[i][j]);
		}
		free(m[i]);
	}
	free(m);

	return 1;
}

int *matrixToArray(int **M, int n)
{
	int i, j, k, *A;
	A = (int*)malloc(sizeof(int)*(n*n));

	for(i = 0, k = 0; i < n; i++){
		for(j = 0; j < n; j++, k++){
			A[k] = M[j][i];
		}
	}

	return A;
}

int *computeColumnSums(int *a, int n, int t)
{
	int *sums, divs, i, j, k;

	divs = n/t;

	if(n > divs*t){
		divs++;
	}

	sums = (int*)malloc(sizeof(int)*divs);

	for(i = 0; i < divs; i++){
		sums[i] = 0;
	}

	for(i = 0, j = 0, k = 1; i < (n*divs); i++, k++){
		sums[j] += a[i];
		if(k == n){
			j++;
			k = 0;
		}
	}
	printf("\n");

	return sums;
}