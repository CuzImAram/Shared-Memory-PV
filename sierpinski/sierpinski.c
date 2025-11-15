#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

// default values
#define IMG_SIZE_DEFAULT 5000
#define NUM_SAMPLES_DEFAULT 10000
#define MAX_ITER_DEFAULT 10000
#define MAX_DIFF_DEFAULT 0.0005

#define max(X, Y)  ((X) > (Y) ? (X) : (Y))
#define abs(X)  ((X) < 0 ? -(X) : (X))
#define thrd  0.333333


double trafo[8][6] = {
  {thrd, 0.0, 0.0, thrd,    0.0,    0.0},
  {thrd, 0.0, 0.0, thrd,   thrd,    0.0},
  {thrd, 0.0, 0.0, thrd, 2*thrd,    0.0},
  {thrd, 0.0, 0.0, thrd,    0.0,   thrd},
  {thrd, 0.0, 0.0, thrd, 2*thrd,   thrd},
  {thrd, 0.0, 0.0, thrd,    0.0, 2*thrd},
  {thrd, 0.0, 0.0, thrd,   thrd, 2*thrd},
  {thrd, 0.0, 0.0, thrd, 2*thrd, 2*thrd}
};

void transform(double* coords, double* coeff) {
  double x = coords[0];
  double y = coords[1];

  coords[0] = x * coeff[0] + y * coeff[1] + coeff[4];
  coords[1] = x * coeff[2] + y * coeff[3] + coeff[5];
}

// Method to write the output image
void export_pgm(int* buffer, size_t img_size, double max) {
  double scale = 256.0 / max;
    
  FILE *fp = fopen("out/sierpinski.pgm", "w");
  fprintf(fp, "P5\n%d %d\n255\n", img_size, img_size);
  
  for (int i = 0; i < img_size*img_size; i++) {
    fprintf(fp, "%c", (char) (buffer[i] * scale));
  }
  
  fclose(fp);
}


int main(int argc, char** argv) {

  int size = argc > 1 ? atoi(argv[1]) : IMG_SIZE_DEFAULT;
  int num_samples = argc > 2 ? atoi(argv[2]) : NUM_SAMPLES_DEFAULT;
  int max_iter = argc > 3 ? atoi(argv[3]) : MAX_ITER_DEFAULT;
  double max_diff = argc > 4 ? atof(argv[4]) : MAX_DIFF_DEFAULT;
  
  int* pixels = calloc(size * size, sizeof(int));
  double* buffer = calloc(size * size / 100, sizeof(double));
  double* samples = calloc(num_samples * 2, sizeof(double));

  double t_start = omp_get_wtime();
  
  //initialize samples
  #pragma omp parallel for
  for (int i = 0; i < num_samples; i++) {
    samples[2*i] = (rand() % 10000) / 10000.0;
    samples[2*i+1] = (rand() % 10000) / 10000.0;

    // "burn in" phase
    for (int j = 0; j < 20; ++j) {
      transform(&samples[i], trafo[rand() % 8]);
    }
  }

  int iter = 0;
  double diff = max_diff + 1.0;
  double max_hits = 0;
  double total_hits = 0.0;
  
  while (diff > max_diff && iter < max_iter) {
    diff = 0.0;

    // Computing the new samples for this round
    #pragma omp parallel for reduction(+:total_hits)
    for (int i = 0; i < num_samples; i++) {
      //Compute the next sample
      transform(&samples[2*i], trafo[rand() % 8]);

      // Scale sample to image coordinates
      int u = (samples[2*i] * (double) size);
      int v = (samples[2*i + 1] * (double) size);

      // Count for each pixel how often it is hit by a sample
      //  and the total number of pixels hits in each round
      if (0 <= u && u < size && 0 <= v && v < size) {
	      size_t idx = (size_t)u + (size_t)v * (size_t)size;
	      #pragma omp atomic
	      ++pixels[idx];
	      total_hits += 1.0;
      }
    }

    // Abort criterion:
    // In buffer we count the number of hits per 10*10 pixels.
    // This buffer will be used to compute the difference between the image in
    // the previous iteration and this iteration. Once the total difference
    // between the falls below max_diff (or we had max_iter iterations),
    // we exit the loop
    int block_dim = size / 10;
    double iteration_max_hits = 0.0;

    #pragma omp parallel for collapse(2) reduction(+:diff) reduction(max:iteration_max_hits)
    for (int col_buf = 0; col_buf < block_dim; col_buf++) {
      for (int row_buf = 0; row_buf < block_dim; row_buf++) {
	      size_t idx_buf = row_buf + col_buf*block_dim;
	      double old = buffer[idx_buf];
	      double block_hits = 0.0;

	      for (int col = 0; col < 10; col++) {
	        for (int row = 0; row < 10; row++) {
	          size_t idx = (10 * row_buf + row) + (10 * col_buf + col) * size;
	          block_hits += pixels[idx];
	          iteration_max_hits = max(iteration_max_hits, (double)pixels[idx]);
	        }
	      }

	      double normalized = block_hits / total_hits;
	      buffer[idx_buf] = normalized;
	      diff += abs(normalized - old);
      }
    }

    max_hits = max(max_hits, iteration_max_hits);
    ++iter;
  }


  double t_end = omp_get_wtime();
  
  printf("Total time: %.3f seconds\n", t_end - t_start);
  printf("#iterations: %d\n", iter);
  printf("Total #samples: %d\n", iter * num_samples);
  printf("Remaining error: %.6f\n", diff);
  export_pgm(pixels, size, max_hits);

  free(pixels);
  free(buffer);
  free(samples);
  return 0;
}
