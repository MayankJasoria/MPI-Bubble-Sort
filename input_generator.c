#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv) {
    if(argc < 2) {
        fprintf(stderr, "Expected 2 arguments, found %d\nUsage: %s <out_file> <num_values>\n", argc - 1, argv[0]);
        return 0;
    }

    FILE* fptr = fopen(argv[1], "w");
    if(fptr == NULL) {
        fprintf(stderr, "Failed to open file for writing\n");
        return 0;
    }

    srand(time(0));

    int num_vals = atoi(argv[2]);

    for(int i = 0; i < num_vals; i++) {
        int val = (rand() % num_vals) + 1;
        fprintf(fptr, "%d\n", val);
    }

    fclose(fptr);

    return 0;
}