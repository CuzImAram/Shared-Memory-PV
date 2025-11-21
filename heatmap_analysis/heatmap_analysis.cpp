#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
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
#pragma omp parallel for
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < columns; ++j)
        {
            unsigned int my_seed = seed * concatenate(i, j);

            // Verwendung von reentrant Zufallszahlengeneratoren -> thread-sicher anders als rand
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

void local_hotspots(unsigned long **A, int rows, int columns, unsigned int work_factor, bool verbose)
{
    int *hotspots_per_row = new int[rows]();
    int total_hotspots = 0;

#pragma omp parallel for reduction(+ : total_hotspots)
    for (int i = 0; i < rows; ++i)
    {
        int row_hotspots = 0;
        for (int j = 0; j < columns; ++j)
        {
            unsigned long center = apply_hash_times(A[i][j], work_factor);
            bool is_hotspot = true;

            // Check up neighbor
            if (i > 0)
            {
                unsigned long up = apply_hash_times(A[i - 1][j], work_factor);
                if (center <= up)
                    is_hotspot = false;
            }

            // Check down neighbor
            if (i < rows - 1 && is_hotspot)
            {
                unsigned long down = apply_hash_times(A[i + 1][j], work_factor);
                if (center <= down)
                    is_hotspot = false;
            }

            // Check left neighbor
            if (j > 0 && is_hotspot)
            {
                unsigned long left = apply_hash_times(A[i][j - 1], work_factor);
                if (center <= left)
                    is_hotspot = false;
            }

            // Check right neighbor
            if (j < columns - 1 && is_hotspot)
            {
                unsigned long right = apply_hash_times(A[i][j + 1], work_factor);
                if (center <= right)
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

    // Output
    if (verbose)
    {
        printf("Hotspots per row:\n");
        for (int i = 0; i < rows; ++i)
        {
            printf("Row %d: %d hotspot(s)\n", i, hotspots_per_row[i]);
        }
    }
    printf("Total hotspots found: %d\n", total_hotspots);

    delete[] hotspots_per_row;
}

void sliding_sums(unsigned long **A, int rows, int columns, unsigned int h, unsigned int work_factor, bool verbose)
{
    // Sicherheitshalber: Wenn das Fenster größer als die Matrix ist, Abbruch
    if (h > rows)
    {
        fprintf(stderr, "Error: window height is greater than rows.\n");
        return;
    }

    // Array zum Speichern der Ergebnisse pro Spalte (damit die Ausgabe geordnet bleibt)
    unsigned long *max_sums = new unsigned long[columns];

// Wir parallelisieren über die SPALTEN (j), da diese unabhängig sind.
#pragma omp parallel for
    for (int j = 0; j < columns; ++j)
    {
        unsigned long current_sum = 0;

        // 1. Initialisierung: Summe des ersten Fensters (Zeile 0 bis h-1)
        for (int k = 0; k < h; ++k)
        {
            current_sum += apply_hash_times(A[k][j], work_factor);
        }

        unsigned long max_sum = current_sum;

        // 2. Sliding Window: Durchlaufe den Rest der Spalte
        // i ist der Startindex des NÄCHSTEN Fensters.
        // Das Fenster geht von i bis i + h - 1
        for (int i = 1; i <= rows - h; ++i)
        {
            // Formel: Neue Summe = Alte Summe - (Element, das rausfällt) + (Element, das reinkommt)
            // Raus: A[i-1][j]
            unsigned long value_out = A[i - 1][j];
            value_out = apply_hash_times(value_out, work_factor);

            // Rein: A[i+h-1][j]
            unsigned long value_in = A[i + h - 1][j];
            value_in = apply_hash_times(value_in, work_factor);

            current_sum = current_sum - value_out + value_in;

            if (current_sum > max_sum)
            {
                max_sum = current_sum;
            }
        }

        // Speichere das Maximum für diese Spalte
        max_sums[j] = max_sum;
    }

    // 3. Sequenzielle Ausgabe (damit die Reihenfolge stimmt: Spalte 0, 1, 2...)
    if (verbose)
    {
        printf("Max sliding sums per column:\n");

        for (int j = 0; j < columns; ++j)
        {
            printf("%lu%s", max_sums[j], (j + 1 == columns) ? ", " : " ");
        }
        printf("\n\n");
    }

    // Aufräumen
    delete[] max_sums;
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
    printf("Parameters: columns=%d, rows=%d, seed=%d, lower=%d, upper=%d, window_height=%d, verbose=%d, num_threads=%d, work_factor=%d\n",
           columns, rows, seed, lower, upper, window_height, verbose, num_threads, work_factor);

    double start_time = omp_get_wtime();

    unsigned long **A = new unsigned long *[rows];
    for (int i = 0; i < rows; ++i)
    {
        A[i] = new unsigned long[columns];
    }

    init_array_parallel(A, rows, columns, seed, lower, upper);

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

    // init test against seq
    ///////////////////////////////////////////////////////////////////////////////

    // double start_time = omp_get_wtime();
    // init_array_seq(A, rows, columns, seed, lower, upper);
    // double end_time = omp_get_wtime();
    // printf("Sequential initialization time: %f seconds\n", end_time - start_time);

    // for (int j = 0; j < columns; ++j)
    // {
    //     printf("%lu%s", A[rows - 1][j], (j + 1 == columns) ? "" : " ");
    // }
    // printf("\n");

    // start_time = omp_get_wtime();
    // init_array_parallel(A, rows, columns, seed, lower, upper);
    // end_time = omp_get_wtime();
    // printf("Parallel initialization time: %f seconds\n", end_time - start_time);

    // for (int j = 0; j < columns; ++j)
    // {
    //     printf("%lu%s", A[rows - 1][j], (j + 1 == columns) ? "" : " ");
    // }
    // printf("\n");

    // Teil 1: Sliding Window Sums
    ///////////////////////////////////////////////////////////////////////////////

    sliding_sums(A, rows, columns, window_height, work_factor, verbose);

    // Teil 2: Local Hotspots
    ///////////////////////////////////////////////////////////////////////////////

    local_hotspots(A, rows, columns, work_factor, verbose);

    // Execution time
    ///////////////////////////////////////////////////////////////////////////////

    double end_time = omp_get_wtime();
    printf("Execution took %.4fs\n", end_time - start_time);

    // Cleanup
    for (int i = 0; i < rows; ++i)
    {
        delete[] A[i];
    }
    delete[] A;

    return 0;
}