#include <omp.h>
#include <math.h>
#include <stdio.h>
#include <limits.h>

#define SIZE 1000

void task1() {
    float x[SIZE], y[SIZE];
    float total = 0.0;
    x[0] = 0;
#pragma omp parallel for
    for (int idx = 1; idx < SIZE; idx++) {
        x[idx] = 2.0 * idx * (idx - 1);
        y[idx] = x[idx] - x[idx - 1];
    }
    for (int i = 1; i < SIZE; i++) {
        total += y[i];
    }
    printf("task1 : total: %f, expected: %f\n", total, (x[SIZE - 1] - x[0]));
}

void task2() {
    int values[SIZE];
    printf("task2 : ");

#pragma omp parallel for
    for (int i = 0; i < SIZE; i++) {
        values[i] = 2 * i;
        if (i % 200 == 0) {
            printf("%d ", values[i]);
        }
    }
    printf(", expected: 0 400 800 ...\n");
}

void task3() {
    int evens = 0;
#pragma omp parallel for
    for (int i = 0; i < SIZE; i++) {
        if (i % 2 == 0) {
            evens++;
        }
    }
    printf("task3 : even count: %d, expected: 500\n", evens);
}

void task4() {
    int max_val = INT_MIN;
#pragma omp parallel for lastprivate(max_val)
    for (int i = 0; i < SIZE; i++) {
        if (i > max_val) {
            max_val = i;
        }
    }
    printf("task4 : max: %d, expected: 999\n", max_val);
}

void task5() {
    float a[SIZE], b[SIZE];
    float sum = 0.0;
    a[0] = 0;
#pragma omp parallel
    {
#pragma omp for nowait
        for (int i = 1; i < SIZE; i++) {
            a[i] = 3.0 * i * (i + 1);
        }
#pragma omp for
        for (int i = 1; i < SIZE; i++) {
            b[i] = a[i] - a[i - 1];
        }
    }
    for (int i = 1; i < SIZE; i++) {
        sum += b[i];
    }
    printf("task5 : result: %f, expected: %f\n", sum, a[SIZE - 1] - a[0]);
}

void task6() {
    float a[SIZE], b[SIZE], x, result, result_expected;
    int i;
    a[0] = 0;
    for (i = 1; i < SIZE; i++) {
        a[i] = 3.0 * i * (i + 1);
        b[i] = a[i] - a[i - 1];
    }
    result_expected = a[SIZE - 1] - a[0];
#pragma omp parallel for
    for (i = 1; i < SIZE; i++) {
        x = sqrt(b[i]) - 1;
        a[i] = x * x + 2 * x + 1;
    }

    result = 0;
    for (i = 1; i < SIZE; i++) {
        result += a[i];
    }
    printf("task6 : result: %f, expected: %f\n", result, result_expected);
}

void task7() {
#pragma omp parallel
    {
        int id = omp_get_thread_num();
#pragma omp single
        printf("task7 : thread ID: %d, expected: 0\n", id);
    }
}

void task8() {
    float x = 0, y = 0;
    omp_lock_t lock_x, lock_y;
    omp_init_lock(&lock_x);
    omp_init_lock(&lock_y);
#pragma omp parallel for shared(x, y)
    for (int i = 1; i <= SIZE; i++) {
        if (i < 0.3 * SIZE) {
            omp_set_lock(&lock_y);
            y += i;
            omp_set_lock(&lock_x);
            x += i;
            omp_unset_lock(&lock_x);
            omp_unset_lock(&lock_y);
        } else {
            omp_set_lock(&lock_x);
            x += i;
            omp_set_lock(&lock_y);
            y += i;
            omp_unset_lock(&lock_y);
            omp_unset_lock(&lock_x);
        }
    }
    printf("task8 : x=%.1f, y=%.1f, expected=%.1f\n", x, y, (SIZE + 1.0) * SIZE / 2.0);
}

int main(void) {
    task1();
    task2();
    task3();
    task4();
    task5();
    task6();
    task7();
    task8();
    return 0;
}
