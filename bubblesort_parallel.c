#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Finds a pivot element based on the 'best of 3' approach: 
 * returns the index among start, mid and end of arrays,
 * whose value lies in between the middle of the other two
 * @param arr   The array whose pivot is to be found
 * @param start The starting index of the subarray
 * @param end   The ending index of the subarray
 * 
 * @return The index of the pivot element
 */
int pivot(int* arr, int start, int end) {
	int mid = end - (end-start)/2;
	if(arr[start] <= arr[mid] && arr[start] <= arr[end]) {
		if(arr[mid] <= arr[end]) {
			/* arr[start] <= arr[mid] <= arr[end]*/
			return mid;
		} else {
			/* arr[start] <= arr[end] < arr[mid] */
			return end;
		}
	} else if(arr[mid] <= arr[start] && arr[mid] <= arr[end]) {
		if(arr[start] <= arr[end]) {
			/* arr[mid] <= arr[start] <= arr[end] */
			return start;
		} else {
			/* arr[mid] <= arr[end] < arr[start] */
			return end;
		}
	} else {
		if(arr[mid] <= arr[start]) {
			/* arr[end] < arr[mid] <= arr[start] */
			return mid;
		} else {
			/* arr[end] < arr[start] < arr[mid] */
			return start;
		}
	}
}

/**
 * swaps the contents of the array at two given indices
 * @param arr	The array whose contents are to eb swapped
 * @param i1	Index of one element
 * @param i2	Index ofn the other element
 */
void swap(int* arr, int i1, int i2) {
	int temp = arr[i1];
	arr[i1] = arr[i2];
	arr[i2] = temp;
}

/**
 * Partitions the array by a pivot element, by placing all elements less
 * than the pivot before it, and all elements greater than the pivot after
 * it. Therefore, the pivot is in its sorted position after partitioning
 * @param arr   The array to be sorted
 * @param start The starting index of the subarray to be sorted
 * @param end	The ending index of the subarray to be sorted
 * @param left	The index of pivot. Updated to left end of pivot index
 * 				after partitioning
 * @param right	The index of pivot. Updated to right end of pivot index
 * 				after partitioning
 */
void partition(int* arr, int start, int end, int* left, int* right) {
	int piv = arr[*left];
	*left = start;
	*right = start;
	int ubound = end;

	while(*right <= ubound) {
		if(arr[*right] < piv) {
		    /* element is less than pivot */
			swap(arr, *left, *right);
			*left += 1;
			*right += 1;
		} else if(arr[*right] > piv) {
		    /* element is greater than pivot */
			swap(arr, *right, ubound);
			ubound--;
		} else {
			/* element is equal to pivot */
			*right += 1;
		}
	}
}

/**
 * Sorts the given array using quicksort algorithm (handles duplicates)
 * @param arr   The array to be sorted
 * @param start The starting index of the subarray to be sorted
 * @param end	The ending index of the subarray to be sorted
*/
void quicksort(int* arr, int start, int end) {
	if(start >= end) {
		return;
	}
	int left = pivot(arr, start, end), right = left;
	partition(arr, start, end, &left, &right);
	quicksort(arr, start, left-1);
	quicksort(arr, right, end);
}

/**
 * Merges two sorted arrays into a single sorted array
 * @param arr1	The first sorted array
 * @param size1	Size of the first sorted array
 * @param arr2	The second sorted array
 * @param size2	Size of the seocnd sorted array
 * 
 * @return A sorted array of size (size1 + size2)
 */
int* merge(int* arr1, int size1, int* arr2, int size2) {
	int first = 0;
	int second = 0;
	int sort_index = 0;
	int* sorted;
	MPI_Alloc_mem(sizeof(int) * (size1 + size2), MPI_INFO_NULL, &sorted);
	while(first < size1 && second < size2) {
		if(arr1[first] < arr2[second]) {
			sorted[sort_index++] = arr1[first++];
		} else {
			sorted[sort_index++] = arr2[second++];
		}
	}

	/* copying any remaining elements */
	while(first < size1) {
		sorted[sort_index++] = arr1[first++];
	}

	while(second < size2) {
		sorted[sort_index++] = arr2[second++];
	}

	return sorted;
}

/**
 * Computes the next gap between elements for comparison given the current gap.
 * Required for efficiently merging two sorted arrays in O(1) extra space
 * @param gap	The current gap between two elements to be compared
 * 
 * @return The new gap to be used for comparison
 */
int next_gap(int gap) {
	if(gap <= 1) {
		return 0;
	}
	return (gap / 2) + (gap % 2);
}

/**
 * Performs compare and exchange of data among two given processes so that all
 * elements of one process become smaller than all elements of the other process
 * @param self_arr	The array of the current process
 * @param size		The size of array contained in current process
 * @param self_id	The rank of current process
 * @param rank1		The process which should have all smaller elements
 * @param rank2		The process which should have all larger elements
 */
void compare_split(int* self_arr, int size, int self_id, int rank1, int rank2) {
	/* both processes send their data to the other one (assume same size of both) */
	int other_arr[size]; /* does not need to persist beyond this function */
	MPI_Status status;
	MPI_Sendrecv(
		self_arr,
		size,
		MPI_INT, 
		((self_id == rank1) ? rank2 : rank1),
		0,
		other_arr,
		size,
		MPI_INT,
		((self_id == rank1) ? rank2 : rank1),
		0,
		MPI_COMM_WORLD, 
		&status
	);
	/* TODO: Check for error */

	/*
		invoke algorithm to merge two sorted arrays using O(1) extra space
		reference: https://www.geeksforgeeks.org/efficiently-merging-two-sorted-arrays-with-o1-extra-space/
	*/
	int i, j, gap = size + size;

	/* Possible improvement below: merge the if-else branches, ignore sorting of non-required array */
	if(self_id == rank1) {
		/* self_arr stores smallest elements in sorted order */
		for (gap = next_gap(gap); gap > 0; gap = next_gap(gap)) { 
			/* comparing elements in the first array */ 
			for (i = 0; i + gap < size; i++) {
				if (self_arr[i] > self_arr[i + gap]) { 
					swap(self_arr, i, i + gap); 
				}
			}
	
			/* comparing elements in both arrays */
			for (j = gap > size ? gap-size : 0 ; i < size && j < size; i++, j++) { 
				if (self_arr[i] > other_arr[j]) {
					/* swap contents of the two arrays at indices i and j respectively */ 
					int temp = self_arr[i];
					self_arr[i] = other_arr[j];
					other_arr[j] = temp;
				}
			}
	
			if (self_id == rank2 && j < size) { 
				/* comparing elements in the second array */ 
				for (j = 0; j + gap < size; j++) { 
					if (other_arr[j] > other_arr[j + gap]) {
						swap(other_arr, j, j + gap);
					}
				}
			} 
		}
	} else {
		/* self_arr stores largest elements, sorted in ascending order */
				/* self_arr stores smallest elements in sorted order */
		for (gap = next_gap(gap); gap > 0; gap = next_gap(gap)) { 
			/* comparing elements in the first array */ 
			for (i = 0; i + gap < size; i++) {
				if (other_arr[i] > other_arr[i + gap]) { 
					swap(other_arr, i, i + gap); 
				}
			}
	
			/* comparing elements in both arrays */
			for (j = gap > size ? gap-size : 0 ; i < size && j < size; i++, j++) { 
				if (other_arr[i] > self_arr[j]) {
					/* swap contents of the two arrays at indices i and j respectively */ 
					int temp = other_arr[i];
					other_arr[i] = self_arr[j];
					self_arr[j] = temp;
				}
			}
	
			if (self_id == rank2 && j < size) { 
				/* comparing elements in the second array */ 
				for (j = 0; j + gap < size; j++) { 
					if (self_arr[j] > self_arr[j + gap]) {
						swap(self_arr, j, j + gap);
					}
				}
			} 
		}
	}
}

int  main(int argc, char** argv) {
	
	int num_proc;
	int id;
	int size;
	int chunk_size;
	int* arr;
	int* chunk;
	double time_taken;

	/* Initialize MPI */
	MPI_Init(&argc, &argv);

	/* Get number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

	/* Get current process id */
	MPI_Comm_rank(MPI_COMM_WORLD, &id);

	if(id == 0) {
		/* read imput file */
		// /* assumption: input file is always input.txt, output file is always output.txt */
		// FILE* fp = fopen("input.txt", "r");

		/* Open the input file specified as command line argument */
		FILE* fp = fopen(argv[1], "r");
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
		MPI_Alloc_mem(chunk_size * num_proc * sizeof(int), MPI_INFO_NULL, &arr);

		/* read data from file */
		int i;
		for(i = 0; i < size; i++) {
			fscanf(fp, "%d", &arr[i]);
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
	MPI_Alloc_mem(chunk_size * sizeof(int), MPI_INFO_NULL, &chunk);

	MPI_Scatter(arr, chunk_size, MPI_INT, chunk, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

	/* no need to maintain original array */
	if(id == 0) {
		MPI_Free_mem(arr);
	}

	/* sort the chunk of data in each process */
	quicksort(chunk, 0, chunk_size-1);

	/* Synchronizing processes post individual sorting */
	MPI_Barrier(MPI_COMM_WORLD);

	/* perform shell sort */
	int i = 1;
	// int partition_size = num_proc;
	// for(i = 1; i < num_proc; i *= 2, ((partition_size % 2 == 0) ? (partition_size /= 2) : (partition_size = (partition_size / 2) + 1))) { // log_2 (P) steps
	// 	/* compute rank of other process */
	// 	int diff = id - ((id / partition_size) * (partition_size));
	// 	int other_rank = ((id / partition_size) + 1) * partition_size - (diff + 1);
	// 	if(other_rank > id) {
	// 		compare_split(chunk, chunk_size, id, id, other_rank);
	// 	} else {
	// 		compare_split(chunk, chunk_size, id, other_rank, id);
	// 	}
	// 	/* Synchronize may be needed after each iteration: uncomment below to synchronize */
	// 	MPI_Barrier(MPI_COMM_WORLD);
	// }

	/* Perform odd-even transposition */
	/* TODO: find a way to limit number of iterations */
	// char fname[11];
	// strcpy(fname, "debug");
	// fname[5] = (char) id + '0';
	// fname[6] = '.';
	// fname[7] = 't';
	// fname[8] = 'x';
	// fname[9] = 't';
	// fname[10] = '\0';
	// FILE* debugfile = fopen(fname, "w");
	for(i = 0; i < num_proc; i++) {
		if(i%2 == 0) {
			/* even to odd transposition */
			if((id % 2 == 0) && (id < num_proc - 1)) {
				// fprintf(debugfile, "i: %d, self_id: %d, rank1: %d, rank2: %d\n",i, id, id, id+1);
				// fflush(debugfile);
				compare_split(chunk, chunk_size, id, id, id+1);
			} else if (id % 2 == 1) {
				// fprintf(debugfile, "i: %d, self_id: %d, rank1: %d, rank2: %d",i, id, id-1, id);
				// fflush(debugfile);
				compare_split(chunk, chunk_size, id, id-1, id);
			}
		} else {
			/* odd to even transposition */
			if((id % 2 == 1) && (id <= num_proc - 2)) {
				// fprintf(debugfile, "o: %d, self_id: %d, rank1: %d, rank2: %d",i, id, id, id+1);
				// fflush(debugfile);
				compare_split(chunk, chunk_size, id, id, id+1);
			} else if((id %2 == 0) && (id > 0)) {
				// fprintf(debugfile, "i: %d, self_id: %d, rank1: %d, rank2: %d",i, id, id-1, id);
				// fflush(debugfile);
				compare_split(chunk, chunk_size, id, id-1, id);
			}
		}
		/* Synchronize may be needed after each iteration: uncomment below to synchronize */
		MPI_Barrier(MPI_COMM_WORLD);
	}

	// fclose(debugfile);

	/* Sorting ends, collect output */
	// MPI_Barrier(MPI_COMM_WORLD);

	/* sorting ends, merge results into root */
	if(id == 0) {
		MPI_Alloc_mem(num_proc * chunk_size * sizeof(int), MPI_INFO_NULL, &arr);
		memset(arr, 0, chunk_size * num_proc * sizeof(int)); // TODO: Remove this line
	}

	/* aggregate results from all processes */
	int status = MPI_Gather(chunk, chunk_size, MPI_INT, arr, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

	/* allocated memory chunk no longer needed */
	MPI_Free_mem(chunk);

	// MPI_Barrier(MPI_COMM_WORLD);

	if(id == 0) {

		// /* DEBUG */
		printf("Communicator Size: %d\nMPI_Gather status: %d\n", num_proc, status);
		// /* DEBUG ENDS */

		/* print output to file: assume output file name is output.txt */
		FILE* outfile = fopen("output.txt", "w");
		for(i = 0; i < size; i++) {
			fprintf(outfile, "%d\n", arr[i]);
		}
		fclose(outfile);

		printf("Time taken for execution: %f\n", time_taken);
	}

	/* stop timer */
	time_taken = MPI_Wtime() - time_taken;

	/* All processes have shared their data, individual chunks not needed anymore */
	// MPI_Free_mem(chunk);

	/* All operations completed. Clean up MPI state */
	MPI_Finalize();

	return 0;
}