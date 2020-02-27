#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void swap(int* arr, int i1, int i2) {
    int temp = arr[i1];
    arr[i1] = arr[i2];
    arr[i2] = temp;
}

void bubblesort(int* arr, int size) {
    int i, j;
    for(i = 0; i < size-1; i++) {
        for(j = 0; j < size-i-1; j++) {
            if(arr[j] > arr[j+1]) {
                swap(arr, j, j+1);
            }
        }
    }
}

int main(int argc, char** argv) {
    int num_proc;
    int id;
    int size;
    int chunk_size;
    int *arr;
    int *chunk;
    double time_taken;

    /* Initialize MPI */
	MPI_Init(&argc, &argv);

	/* Get number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

	/* Get current process id */
	MPI_Comm_rank(MPI_COMM_WORLD, &id);

	if(id == 0) {
		/* read imput file */
		/* assumption: input file is always input.txt, output file is always output.txt */
		FILE* fp = fopen("input.txt", "r");
		/* assumption: first line specifies number of integers in the file */
		fscanf(fp, "%d", &size);

		/* compute chunk size */
		chunk_size = size / num_proc;
		if(size % num_proc != 0) {
			/*
			increase chunk size to the next multiple of num_proc. Else, the 
			data will not fit in all the processes
			*/
			chunk_size++;
		}

		/* create array of size such that all processes receive equal parts */
		arr = (int*) malloc(chunk_size * num_proc * sizeof(int));

		/* read data from file */
		int i;
		for(i = 0; i < size; i++) {
			fscanf(fp, "%d", arr[i]);
		}
		/* reading is complete, so close the file */
		fclose(fp);

		/* pad array with infinity --> simplified future computations */
		for(i = size; i < chunk_size * num_proc; i++) {
			arr[i] = __INT_MAX__;
		}
	}

    /* synchronize all processes upto this point */
	MPI_Barrier(MPI_COMM_WORLD);

	/* start timer */
	time_taken = MPI_Wtime();

	/* broadcast the chunk size to all processes */
	MPI_Bcast(&chunk_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

	/* scatter the input array to all processes */
	chunk = (int*) malloc(chunk_size * sizeof(int));

	MPI_Scatter(arr, chunk_size, MPI_INT, chunk, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

	/* no need to maintain original array */
	free(arr);

    /* bubblesort the chunk of data in each process */
    bubblesort(chunk, chunk_size);

    /* sorting ends, merge results into root */
	if(id == 0) {
		arr = (int*) malloc(num_proc * chunk_size * sizeof(int));
	}

	MPI_Gather(chunk, chunk_size, MPI_INT, arr, (chunk_size * num_proc), MPI_INT, 0, MPI_COMM_WORLD);

	/* stop timer */
	time_taken = MPI_Wtime() - time_taken;

	if(id == 0) {
		/* print output to file: assume output file name is output.txt */
		FILE* outfile = fopen("output.txt", "w");
		for(i = 0; i < size; i++) {
			fprintf(outfile, "%d\n", arr[i]);
		}
		fclose(outfile);
	}

	MPI_Finalize();
    
    return 0;
}