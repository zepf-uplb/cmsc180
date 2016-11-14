#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

/*int *computeVectorProducts(int rank, int **local, int *V, int n)
{
    int *sum, i, j, k;

    sum = (int*)malloc(sizeof(int)*(n/2));  

    if(rank%2 == 0){
        k = 0;
    }
    else{
        k = n/2;
    }

    for(i = 0; i < n/2; i++){
        sum[i] = 0;
        for(j = 0; j < n/2; j++){
            sum[i] += local[i][j] * V[k+j];
        }
    }

    return sum;
}

int *createVector(int *V, int n)
{
    int i;  

    for(i = 0; i < n; i++){
        V[i] = (rand() % 5)+1;
    }   

    return V;
}*/

int *createArray(int n)
{
    int *A, i, j;

    A = (int*)malloc(sizeof(int)*n*n);

    for(i = 0; i < n*n; i++){
        A[i] = (rand() % 5)+1;
    }   

    return A;
}    

int malloc2D(int ***array, int n, int m) {
    int i;

    int *p = malloc(n*m*sizeof(int));
    if (!p) return -1;

    (*array) = malloc(n*sizeof(int*));
    if (!(*array)) {
       free(p);
       return -1;
    }

    for (i=0; i<n; i++)
       (*array)[i] = &(p[i*m]);

    return 0;
}

int free2D(int ***array) {
    free(&((*array)[0][0]));
    free(*array);
    return 0;
}

int main(int argc, char **argv) {
    int **global, **local;
    const int n = 4; 
    const int grid = 2;  
    int rank, size;     
    int i, j, p;
    int *A, *a;
    int color;
    //int *PRODUCT;
    clock_t start, stop;
    double cpu_time_used;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    srand(time(NULL));

    color = rank / 2;

    MPI_Comm newComm;
    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &newComm);

    int newRank, newSize;
    MPI_Comm_rank(newComm, &newRank);
    MPI_Comm_size(newComm, &newSize);

    color = rank % 2;

    MPI_Comm newComm2;
    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &newComm2);

    int newRank2, newSize2;
    MPI_Comm_rank(newComm2, &newRank2);
    MPI_Comm_size(newComm2, &newSize2);

    a = (int*)malloc(sizeof(int)*((n*n)/2));
    if(rank == 0 || rank == 2){
        A = (int*)malloc(sizeof(int)*((n*n)));
    }

    if (size != grid*grid) {
        fprintf(stderr,"%s: Only works with np=%d for now\n", argv[0], grid);
        MPI_Abort(MPI_COMM_WORLD,1);
    }

    if (rank == 0) {

        malloc2D(&global, n, n);

        for (i=0; i<n; i++) {
            for (j=0; j<n; j++)
                global[i][j] = (rand() % 5)+1;
        }

        printf("Global array is:\n");
        for (i=0; i<n; i++) {
            for (j=0; j<n; j++) {
                printf("%2d ", global[i][j]);
            }
            printf("\n");
        }

        A = createArray(n);
        int *temp = (int*)malloc(sizeof(int)*n*n);
        int k;
        printf("Matrix B:");
        for(i = 0; i < n*n; i++){
            if(i%n == 0) printf("\n");
            printf("%d ", *(A+i));
        }
        printf("\n");
        //transpose stuff
        for(i = 0, j = 0, k = 0; i < n*n; i++){
            temp[i] = A[j];
            //printf("temp[%d] = %d, A[%d] = %d\n", i, temp[i], j, A[j]);           
            j += n;
            if(j >= n*n){
                k++;
                j = k;
            }
        }
        free(A);
        A = temp;
        /*printf("Matrix B:");
        for(i = 0; i < n*n; i++){
            if(i%n == 0) printf("\n");
            printf("%d ", *(A+i));
        }
        printf("\n");*/
    }

    malloc2D(&local, n/grid, n/grid);

    int sizes[2]    = {n, n};         
    int subsizes[2] = {n/grid, n/grid};     
    int starts[2]   = {0,0};               
    MPI_Datatype type, subarrtype;
    MPI_Type_create_subarray(2, sizes, subsizes, starts, MPI_ORDER_C, MPI_INT, &type);
    MPI_Type_create_resized(type, 0, n/grid*sizeof(int), &subarrtype);
    MPI_Type_commit(&subarrtype);

    int *globalptr=NULL;
    if (rank == 0)
        globalptr = &(global[0][0]);

    int sendcounts[grid*grid];
    int displs[grid*grid];

    if (rank == 0) {
        for (i=0; i<grid*grid; i++)
            sendcounts[i] = 1;
        int disp = 0;
        for (i=0; i<grid; i++) {
            for (j=0; j<grid; j++) {
                displs[i*grid+j] = disp;
                disp += 1;
            }
            disp += ((n/grid)-1)*grid;
        }
    }

    if(rank == 0){
        start = clock();
    }

    MPI_Scatterv(globalptr, sendcounts, displs, subarrtype, &(local[0][0]),
                 n*n/(grid*grid), MPI_INT,
                 0, MPI_COMM_WORLD);


    for (p=0; p<size; p++) {
        if (rank == p) {
            printf("Local process on rank %d is:\n", rank);
            for (i=0; i<n/grid; i++) {
                putchar('|');
                for (j=0; j<n/grid; j++) {
                    printf("%2d ", local[i][j]);
                }
                printf("|\n");
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    if(rank == 0 || rank == 2){
        MPI_Bcast(A, n*n, MPI_INT, 0, newComm2);
    }

    MPI_Scatter(A, (n*n)/2, MPI_INT, a, (n*n)/2, MPI_INT, 0, newComm);

    if(rank == 0 || rank == 2){
        free(A);  
    }

    for (p=0; p<size; p++) {
        if (rank == p) {
            printf("Local B matrix on rank %d is:", rank);
            for(i = 0; i < (n*n)/2; i++){
                if(i%n == 0) printf("\n");
                printf("%d ", a[i]);
            }
            printf("\n");
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    

    //MPI_Bcast(V, n, MPI_INT, 0, MPI_COMM_WORLD);

    //sum = computeVectorProducts(rank, local, V, n);

    

    //MPI_Reduce(sum, SUM, n/2, MPI_INT, MPI_SUM, 0, newComm);    

    /*if(rank == 0 || rank == 2){
        MPI_Gather(SUM, n/2, MPI_INT, PRODUCT, n/2, MPI_INT, 0, newComm2);
        free(SUM);
    }*/   
    
    if (rank == 0) {
        stop = clock();
        cpu_time_used = (double)(stop - start)/(double) CLOCKS_PER_SEC;
        //printf("Time elapsed: %f \n", cpu_time_used);   
        free2D(&global);
        //free(A);
        //free(PRODUCT);
    }
    free(a);
    free2D(&local);
    MPI_Type_free(&subarrtype);

    //free(V);
    //free(sum);

    MPI_Comm_free(&newComm);
    MPI_Comm_free(&newComm2);

    MPI_Finalize();

    return 0;
}
