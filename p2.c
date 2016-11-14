#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <stdlib.h>

int **createMatrix(int n);
int *createVector(int *V, int n);
int printMatrix(int **M, int n);
int printArray(int *A, int n);
int freeMatrix(int **M, int n);
int *matrixToArray(int **M, int n);
int *computeVectorProducts(int *a, int *v, int *scounts, int n, int myrank);

int main(int argc, char** argv)
{
    int myrank, nprocs, **M, *A, *a, n, t, i, j, divs, a_size;
    int *displs, *scounts, stride, *rcounts, *displs_r, *V, *answerVector;
    clock_t start, stop;
	double cpu_time_used;

    n = 10000, t = 4;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    srand(time(NULL));

    V = (int*)malloc(sizeof(int)*n);

    if(myrank == 0){
    	M = createMatrix(n);      
    	V = createVector(V, n);
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
    answerVector = (int*)malloc(sizeof(int)*n);

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

    for(; i < t; i++){
    	if(i != 0){
    		displs_r[i] = rcounts[i-1] + displs_r[i-1]; 
    	}
    	else{
    		displs_r[i] = 0;
    	}	
    	rcounts[i] = (scounts[i]/n);
    } 

    a = (int*)malloc(sizeof(int)*(n*a_size));

    if(myrank == 0){
    	start = clock();
    }

    MPI_Scatterv(A, scounts, displs, MPI_INT, a, (n*a_size), MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Bcast(V, n, MPI_INT, 0, MPI_COMM_WORLD);    

    V = computeVectorProducts(a, V, scounts, n, myrank);   
    
    MPI_Gatherv(V, (scounts[myrank]/n), MPI_INT, answerVector, rcounts, displs_r, MPI_INT, 0, MPI_COMM_WORLD);    

    if(myrank == 0){
    	stop = clock();
    	cpu_time_used = (double)(stop - start)/(double) CLOCKS_PER_SEC;
		printf("Time elapsed: %f \n", cpu_time_used);		
	    free(A);
	    free(answerVector);
    }
    free(V);
    free(displs);
    free(scounts);
    free(rcounts);
    free(displs_r);
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

int *createVector(int *V, int n)
{
	int i;	

	for(i = 0; i < n; i++){
		V[i] = (rand() % 5)+1;
	}	

	return V;
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
	for(i = 0; i < n; i++){
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

int *computeVectorProducts(int *a, int *v, int *scounts, int n, int myrank)
{
	int i, j, k, divs, *sum;

	divs = scounts[myrank] / n;

	sum = (int*)malloc(sizeof(int)*divs);  

	for(i = 0; i < divs; i++){
		sum[i] = 0;
	}

	for(i = 0, j = 0, k = 0; i < (n*divs); i++, k++){
		a[i] *= v[k];
		sum[j] += a[i];
		if(k == n-1){
			j++;
			k = -1;
		}
	}

	for(i = 0; i < divs; i++){
		v[i] = sum[i];
	}

	return v;
}
