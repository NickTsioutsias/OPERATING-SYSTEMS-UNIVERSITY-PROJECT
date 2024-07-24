#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define MAX_PROCESSES 64

double get_wtime(void) {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + (double)t.tv_usec * 1.0e-6;
}

inline double f(double x) {
    return sin(cos(x));
}

// Shared memory structure
typedef struct {
    double partial_sums[MAX_PROCESSES];
} SharedData;


int main(int argc, char *argv[]) {
  
  double a = 0.0;
  double b = 1.0;
  unsigned long n = 24e7;  // or e8
  const double ref = 0.73864299803689018;
  const long tseed = 10;
  double res = 0;
  double t0, t1;  

  return 0;
}
