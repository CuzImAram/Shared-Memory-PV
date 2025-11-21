#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

unsigned concatenate(unsigned x, unsigned y)
{
    unsigned pow = 10;
    while (y >= pow)
    {
        pow *= 10;
    }
    return x * pow + y;
}

void init_array_seq(unsigned long **A, int rows, int columns, unsigned int seed, unsigned int lower, unsigned int upper)
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

void init_array_parallel(unsigned long **A, int rows, int columns, unsigned int seed, unsigned int lower, unsigned int upper)
{
    const int tile_size = 64; // Define the tile size for cache efficiency
    #pragma omp parallel for collapse(2) schedule(dynamic)
    for (int ii = 0; ii < rows; ii += tile_size)
    {
        for (int jj = 0; jj < columns; jj += tile_size)
        {
            // --- START OF TILE ---
            
            // Determine the boundaries of the current tile
            int i_end = (ii + tile_size < rows) ? ii + tile_size : rows;
            int j_end = (jj + tile_size < columns) ? jj + tile_size : columns;

            for (int i = ii; i < i_end; ++i)
            {
                for (int j = jj; j < j_end; ++j)
                {
                    // 2. Thread-safe Random Generation
                    // We calculate the specific seed for this element
                    unsigned int element_seed = seed * concatenate(i, j);
                    
                    // Use rand_r which takes a pointer to the local seed
                    // This avoids global state contention
                    A[i][j] = rand_r(&element_seed) % (upper - lower) + lower;
                }
            }
            // --- END OF TILE ---
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
    // init
    ///////////////////////////////////////////////////////////////////////////////
    if (argc != 10)
    {
        fprintf(stderr, "Usage: %s <columns> <rows> <seed> <lower> <upper>\
            <window_height> <verbose> <num_threads> <work_factor>\n",
                argv[0]);
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

    float start_time = omp_get_wtime();
    init_array_seq(A, rows, columns, seed, lower, upper);
    float end_time = omp_get_wtime();
    printf("Sequential initialization time: %f seconds\n", end_time - start_time);

    for (int j = 0; j < columns; ++j)
    {
        printf("%lu%s", A[rows - 1][j], (j + 1 == columns) ? "" : " ");
    }
    printf("\n");

    start_time = omp_get_wtime();
    init_array_parallel(A, rows, columns, seed, lower, upper);
    end_time = omp_get_wtime();
    printf("Parallel initialization time: %f seconds\n", end_time - start_time);

    ///////////////////////////////////////////////////////////////////////////////

    for (int j = 0; j < columns; ++j)
    {
        printf("%lu%s", A[rows - 1][j], (j + 1 == columns) ? "" : " ");
    }
    printf("\n");

    return 0;
}