#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <stdlib.h>

int **createMatrix(int n);
int printMatrix(int **M, int n);
int printArray(int *A, int n);
int freeMatrix(int **M, int n);
int *matrixToArray(int **M, int n);
int *computeMatrixProducts(int *a, int *v, int *scounts, int n, int myrank);

int main(int argc, char** argv)
{
    int myrank, nprocs, **M, *A, *B, *C, *a, n, t, i, j, divs, a_size;
    int *displs, *scounts, stride, *rcounts, *displs_r;
    clock_t start, stop;
	double cpu_time_used;

    n = 10000, t = 4;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    srand(time(NULL));

    B = (int *)malloc(sizeof(int)*n*n); 

    if(myrank == 0){
    	C = (int*)malloc(sizeof(int)*(n*n));
    }

    if(myrank == 0){
    	M = createMatrix(n);    	
    	A = matrixToArray(M, n);
    	freeMatrix(M, n);

    	M = createMatrix(n);
    	B = matrixToArray(M, n);
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

    a = (int*)malloc(sizeof(int)*(n*a_size));

    if(myrank == 0){
    	start = clock();
    }

    MPI_Scatterv(A, scounts, displs, MPI_INT, a, (n*a_size), MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Bcast(B, n*n, MPI_INT, 0, MPI_COMM_WORLD);        

    B = computeMatrixProducts(a, B, scounts, n, myrank);       
    
    MPI_Gatherv(B, scounts[myrank], MPI_INT, C, scounts, displs, MPI_INT, 0, MPI_COMM_WORLD);  

    if(myrank == 0){
    	stop = clock();
    	cpu_time_used = (double)(stop - start)/(double) CLOCKS_PER_SEC;
		printf("Time elapsed: %f \n", cpu_time_used);		
	    free(A);
	    free(C);
    }
    
    free(B);
    free(displs);
    free(scounts);
    free(a);

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
			M[i][j] = (rand() % 5)+1;
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

int printArray(int *A, int n)
{
	int i;
	for(i = 0; i < n*n; i++){
		if(i%n == 0){
			printf("\n");
		}
		printf("%d, ", A[i]);
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

int *matrixToArray(int **M, int n)
{
	int i, j, k, *A;
	A = (int*)malloc(sizeof(int)*(n*n));

	for(i = 0, k = 0; i < n; i++){
		for(j = 0; j < n; j++, k++){
			A[k] = M[i][j];
		}
	}

	return A;
}

int *computeMatrixProducts(int *a, int *B, int *scounts, int n, int myrank)
{
	int i, j, k, l, divs, *C;	

	divs = scounts[myrank] / n;

	C = (int*)malloc(sizeof(int)*scounts[myrank]);

	for(i = 0; i < scounts[myrank]; i++){
		C[i] = 0;
	}

	for(i = 0, l = 0; i < scounts[myrank]; i++, l=i%n){
		for(j = (i/n)*n, k = 0; k < n; j++, k++, l+=n){
			C[i] += a[j]*B[l];			
		}
	}	

	return C;
}
