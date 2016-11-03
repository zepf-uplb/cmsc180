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
int *computeColumnSums(int *a, int *scounts, int n, int myrank);

int main(int argc, char** argv)
{
    int myrank, nprocs, **M, ***m, *A, *a, n, t, i, j, divs, a_size, *sums, *SUMS, *displs, *scounts;
    int stride, *rcounts, *displs_r;

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

    	freeMatrix(M, n);
    }
	     
    divs = n/t;

	if(n > divs*t){
		a_size = divs+1;
	}
	else{
		a_size = divs;
	}

	displs = (int *)malloc(sizeof(int)*t); 
    scounts = (int *)malloc(sizeof(int)*t);
    rcounts = (int *)malloc(sizeof(int)*n); 
    displs_r = (int *)malloc(sizeof(int)*n); 

    i = 0;
    displs[0] = 0;
    scounts[0] = 0;

    for(stride = n + (n*divs); i < n%t; i++){ 
        displs[i] = i*stride; 
        scounts[i] = n + (n*divs); 
    }

    for(; i < t; i++){
    	if(i != 0){
    		displs[i] = scounts[i-1] + displs[i-1]; 
    	}
    	else{
    		displs[i] = 0;
    	}	
    	scounts[i] = n*divs;
    }   
    i = 0;
    displs_r[0] = 0;
    rcounts[0] = 0;

    for(stride = (scounts[0]/n); i < n%t; i++){
    	displs_r[i] = i*stride;
    	rcounts[i] = stride;

    }

    a = (int*)malloc(sizeof(int)*(n*a_size));
    assert(a != NULL);

    //MPI_Scatter(A, (n*divs), MPI_INT, a, (n*divs), MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatterv(A, scounts, displs, MPI_INT, a, (n*a_size), MPI_INT, 0, MPI_COMM_WORLD);


    if(myrank == 1){
    	printf("I am processor %d.\n", myrank);

    	for(i = 0; i < t; i++){
	    	printf("displs[%d] = %d, scounts[%d] = %d\n", i, displs[i], i, scounts[i]);
	    }	    
	    printf("\n");   

	    for(i = 0; i < t; i++){
	    	printf("displs_r[%d] = %d, rcounts[%d] = %d\n", i, displs_r[i], i, rcounts[i]);
	    }	    
	    printf("\n"); 

    	for(i = 0; i < n*a_size; i++){
    		printf("%d ", a[i]);
    	}	
    	printf("\n");   	
    }

    sums = computeColumnSums(a, scounts, n, myrank);

    if(myrank == 1){
	    for(i = 0; i < scounts[myrank] / n; i++){
	    	printf("%d ", sums[i]);
	    }    	
	    printf("\n");   
    }


    /*printf("I am processor %d. :", myrank);
    for(i = 0; i < divs; i++){
    	printf("%d ", sums[i]);
    }	
    printf("\n");*/

   /* SUMS = (int*)malloc(sizeof(int)*n);
    assert(SUMS != NULL);

    //MPI_Allgather(sums, divs, MPI_INT, SUMS, divs, MPI_INT, MPI_COMM_WORLD);
    MPI_Allgatherv(sums, (scounts[myrank]/n), MPI_INT, SUMS, scounts, displs, MPI_INT, MPI_COMM_WORLD);
	
    if(myrank == 0){
    	printf("\n");
    	for(i = 0; i < n; i++){
	    	printf("%d ", SUMS[i]);
	    }
	    printf("\n");
	    free(A);
    }*/
    free(displs);
    free(scounts);
    free(sums);
    //free(SUMS);
	    

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

/*int ***divideMatrix(int **M, int t, int n)
{
	int ***m, i, j, k, divs;

	divs = n/t;

	if(n > divs*t){
		divs++;
	}

	m = (int***)malloc(sizeof(int**)*t);

	for(i = 0; i < t; i++){
		m[i] = (int**)malloc(sizeof(int*)*n);
		for(j = 0; j < n; j++){
			m[i][j] = (int*)malloc(sizeof(int)*(divs));
		}
	}

	

	for(k = 0; k < t; k++){
		for(i = 0; i < n; i++){
			for(j = 0; j < divs; j++){
				m[k][i][j] = M[i][j+(k*(divs))];
			}
		}
	}

	return m;
}*/

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

/*int *computeColumnSums(int *a, int n, int t)
{
	int *sums, divs, i, j, k, size;

	size = sizeof(a) / sizeof(int);

	divs = n/t;

	if(a[size-1] != 0){
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

	return sums;
}*/

int *computeColumnSums(int *a, int *scounts, int n, int myrank)
{
	int i, j, k, divs, *sums;

	divs = scounts[myrank] / n;

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

	return sums;
}