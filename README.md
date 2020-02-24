# MPI Bubble Sort
Bubble sort using OpenMPI

Odd-even Transposition: Split data into chunks of size n/p. Sort each chunk using quicksort. THen use odd-even scheme to merge data of all p processes, taking exactly p steps.

shell sort: combine-sort Pi and P(p-i-1) processes. At each step, divide the set p into two parts and repear the algorithm, till p=1. THereafter, use odd-even Transposition till no exchanges occur, or till p (initial value) iterations.