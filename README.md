# MPI Bubble Sort
This is a simple program that performs parallel bubble sort on a given array (as file) using OpenMPI. The implementation is broken into the following major steps:

### 1: Splitting the Input
The input is read by one process, which then splits the array into equal sized chunks (padding it with 'infinity' if needed) and sends the chunks to each process.

### 2: Sorting each chunk individually
Each process first sorts the chunk of data that it receives individually, irrespective of the execution or data of another process. This allows every chunk to be sorted in parallel. The given code uses 'serial' quicksort to achieve this, but it may be further improved by using multithreading approaches.

### 3: Odd-Even Transposition
Similar to the merge step of mergesort, this step is about merging two sorted arrays to form one larger sorted array. Instea of making one larger array however, the merging in this case is done without using any extra memory, therefore one array has all elements which are smaller than the other array, and both arrays are individually sorted. However, the technique used here is slightly different as it involves inter-process communication of the chunks.

In the odd-step, processes with an odd rank exchange their chunks with the process having the next higher (even) rank, while in the even step, processes having even ranks communicate their chunks to the processes having the next higher (odd) rank. Once the communication is completed, the processes themselves proform the 'merge' step, referred to in the code as `compare-split`, and stores its respective chunk, which the process having higher rank storing the array with larger values, and the one with lower rank storing the array with smaller values. This operation is performed as many times as the number of processes that are present in the execution.

### 4: Combining the results
At the end of the prevous operation, the results of all processes are combined into one large, sorted array, for producing the required sorted output.

## Possible improvements
Before odd-even transposition, shell-sort may be performed for log p (base 2) no. of iterations, where p is the number of processes. In this technique,  processes at the end exchange data instead of the order used in odd-even transposition, and at the end of each iteration, the ends are divided into two segments, with each segment performing the next steo pf shell-sorting independently of the other segment. Post this, odd-even transposition may be required for lesser number of iterations, to be stopped whenever there is no exchange of data at some step, or till p iterations, whichever occurs earlier. While this technique is slightly slower in the worst case, it turns out to be generally faster. {The code for shell-sort is kept commented in the current file}.

## Executing the code
 * To compile the code, run `mpicc bubblesort_parallel.c`
 * To execute the code (post compilation), run `mpirun -np <no_processes> a.out <input_filepath>`, where `no_processes` is the number of processes that should be launched, and `input_filepath` is the path to the input file.
