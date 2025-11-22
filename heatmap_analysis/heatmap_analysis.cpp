#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <omp.h>

// init
///////////////////////////////////////////////////////////////////////////////

unsigned concatenate(unsigned x, unsigned y)
{
    unsigned pow = 10;
    while (y >= pow)
    {
        pow *= 10;
    }
    return x * pow + y;
}

void init_array(unsigned long **A, int rows, int columns,
                unsigned int seed, unsigned int lower,
                unsigned int upper)
{
#pragma omp parallel for schedule(static) \
    shared(A, rows, columns, seed, lower, upper)
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < columns; ++j)
        {
            unsigned int my_seed = seed * concatenate(i, j);

            // Verwendung von reentrant Zufallszahlengeneratoren
            // -> thread-sicher anders als rand
            struct random_data buf;
            char state_buf[128];
            memset(&buf, 0, sizeof(buf));
            initstate_r(my_seed, state_buf, 128, &buf);
            srandom_r(my_seed, &buf);
            int32_t r;
            random_r(&buf, &r);

            A[i][j] = r % (upper - lower) + lower;
        }
    }
}

// hash
///////////////////////////////////////////////////////////////////////////////

unsigned long hash(unsigned long x)
{
    x ^= (x >> 21);
    x *= 2654435761UL;
    x ^= (x >> 13);
    x *= 2654435761UL;
    x ^= (x >> 17);
    return x;
}

unsigned long apply_hash_times(unsigned long value, unsigned int times)
{
    for (unsigned int w = 0; w < times; ++w)
    {
        value = hash(value);
    }
    return value;
}

void hash_array(unsigned long **A, int rows, int columns,
                unsigned int work_factor)
{
#pragma omp parallel for collapse(2) schedule(static) \
    shared(A, rows, columns, work_factor)
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < columns; ++j)
        {
            A[i][j] = apply_hash_times(A[i][j], work_factor);
        }
    }
}

// Analysis (Sliding Windows + Local Hotspots)
///////////////////////////////////////////////////////////////////////////////

void analyze_heatmap(unsigned long **A, int rows, int columns, unsigned int h,
                     bool verbose)
{
    // Sicherheitshalber: Wenn das Fenster größer als die Matrix ist, Abbruch
    if (h > (unsigned int)rows)
    {
        fprintf(stderr, "Error: window height is greater than rows.\n");
        return;
    }

    unsigned long *max_sums = new unsigned long[columns];
    unsigned long *current_sums = new unsigned long[columns];
    int *hotspots_per_row = new int[rows]();
    int total_hotspots = 0;

#pragma omp parallel shared(A, rows, columns, h, current_sums, max_sums, \
                                hotspots_per_row) reduction(+ : total_hotspots)
    {
// Teil 1: Sliding Window Sums
///////////////////////////////////////////////////////////////////////////////

// 1. Initialisierung: Summe des ersten Fensters (Zeile 0 bis h-1)
#pragma omp for schedule(static) nowait
        for (int j = 0; j < columns; ++j)
        {
            current_sums[j] = 0;
            for (unsigned int k = 0; k < h; ++k)
            {
                current_sums[j] += A[k][j];
            }
            max_sums[j] = current_sums[j];
        }

        // 2. Sliding Window: Durchlaufe den Rest der Spalte
        for (unsigned int i = 1; i <= (unsigned int)rows - h; ++i)
        {
#pragma omp for schedule(static) nowait
            for (int j = 0; j < columns; ++j)
            {
                unsigned long value_out = A[i - 1][j];
                unsigned long value_in = A[i + h - 1][j];

                current_sums[j] = current_sums[j] - value_out + value_in;

                if (current_sums[j] > max_sums[j])
                {
                    max_sums[j] = current_sums[j];
                }
            }
        }

// Teil 2: Local Hotspots
///////////////////////////////////////////////////////////////////////////////
#pragma omp for schedule(static) nowait
        for (int i = 0; i < rows; ++i)
        {
            int row_hotspots = 0;
            for (int j = 0; j < columns; ++j)
            {
                unsigned long center = A[i][j];
                bool is_hotspot = true;

                // Check oberen Nachbarn
                if (i > 0)
                {
                    if (center <= A[i - 1][j])
                        is_hotspot = false;
                }

                // Check unteren Nachbarn
                if (is_hotspot && i < rows - 1)
                {
                    if (center <= A[i + 1][j])
                        is_hotspot = false;
                }

                // Check linken Nachbarn
                if (is_hotspot && j > 0)
                {
                    if (center <= A[i][j - 1])
                        is_hotspot = false;
                }

                // Check rechten Nachbarn
                if (is_hotspot && j < columns - 1)
                {
                    if (center <= A[i][j + 1])
                        is_hotspot = false;
                }

                if (is_hotspot)
                {
                    row_hotspots++;
                }
            }
            hotspots_per_row[i] = row_hotspots;
            total_hotspots += row_hotspots;
        }
    }

    // Output Sliding Sums
    if (verbose)
    {
        printf("Max sliding sums per column:\n");

        for (int j = 0; j < columns; ++j)
        {
            printf("%lu%s", max_sums[j], (j + 1 != columns) ? ", " : " ");
        }
        printf("\n\n");
    }

    // Output Hotspots
    if (verbose)
    {
        printf("Hotspots per row:\n");
        for (int i = 0; i < rows; ++i)
        {
            printf("Row %d: %d hotspot(s)\n", i, hotspots_per_row[i]);
        }
    }
    printf("Total hotspots found: %d\n", total_hotspots);

    // Aufräumen
    delete[] max_sums;
    delete[] current_sums;
    delete[] hotspots_per_row;
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

    printf("Starting heatmap_analysis\n");
    printf("Parameters: columns=%d, rows=%d, seed=%d, lower=%d, upper=%d, "
           "window_height=%d, verbose=%d, num_threads=%d, work_factor=%d\n",
           columns, rows, seed, lower, upper, window_height, verbose,
           num_threads, work_factor);

    double start_time = omp_get_wtime();

    // Einzelner Block of Memory für bessere Cache-Lokalität
    // (Als normales 2D-Array)
    unsigned long *A_storage = new unsigned long[rows * columns];
    unsigned long **A = new unsigned long *[rows];
    for (int i = 0; i < rows; ++i)
    {
        A[i] = &A_storage[i * columns];
    }

    init_array(A, rows, columns, seed, lower, upper);

    if (verbose)
    {
        printf("A:\n");
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < columns; ++j)
            {
                printf("%lu%s", A[i][j], (j + 1 == columns) ? "" : ", ");
            }
            printf("\n");
        }
    }

    // hash array
    hash_array(A, rows, columns, work_factor);

    // Analysis (Sliding Windows + Local Hotspots)
    ///////////////////////////////////////////////////////////////////////////

    analyze_heatmap(A, rows, columns, window_height, verbose);

    // Execution time
    ///////////////////////////////////////////////////////////////////////////
    double end_time = omp_get_wtime();
    printf("Execution took %.4fs\n", end_time - start_time);

    // Cleanup
    delete[] A[0];
    delete[] A;

    return 0;
}