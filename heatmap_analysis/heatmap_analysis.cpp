#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

unsigned concatenate(unsigned x, unsigned y)
{
    unsigned pow = 10;
    while (y >= pow)
    {
        pow *= 10;
        return x * pow + y;
    }
}

void initialize_array(unsigned long **A, int rows, int columns, unsigned int seed, unsigned int lower, unsigned int upper)
{
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < columns; ++j)
        {
            srand(seed * concatenate(i, j));
            A[i][j] = rand() % (upper - lower) + lower;
        }
    }
}

unsigned long hash(unsigned long x)
{
    x ^= (x >> 21);
    x *= 2654435761UL;
    x ^= (x >> 13);
    x *= 2654435761UL;
    x ^= (x >> 17);
    return x;
}

int main(int argc, char **argv)
{
    if (argc < 9)
    {
        fprintf(stderr, "Usage: %s <columns> <rows> <seed> <lower> <upper> <window_height> <verbose> <num_threads> <work_factor>\n", argv[0]);
        return 1;
    }

    int columns = atoi(argv[1]);
    int rows = atoi(argv[2]);
    unsigned int seed = atoi(argv[3]);
    unsigned int lower = atoi(argv[4]);
    unsigned int upper = atoi(argv[5]);
    unsigned int window_height = atoi(argv[6]);
    bool verbose = atoi(argv[7]);
    unsigned int num_threads = atoi(argv[8]);
    unsigned int work_factor = atoi(argv[9]);

    unsigned long **A = new unsigned long *[rows];
    for (int i = 0; i < rows; ++i)
    {
        A[i] = new unsigned long[columns];
    }

    initialize_array(A, rows, columns, seed, lower, upper);

    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < columns; ++j)
        {
            printf("%lu%s", A[i][j], (j + 1 == columns) ? "" : " ");
        }
        printf("\n");
    }

    return 0;
}
