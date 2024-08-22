#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

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
    double *partial_sums;
    int num_processes;
} SharedData;

// WolframAlpha: integral sin(cos(x)) from 0 to 1 = 0.738643
// 0.73864299803689018
// 0.7386429980368901838000902905852160417480209422447648518714116299

int main(int argc, char *argv[]) {
  double a = 0.0;
  double b = 1.0;
  unsigned long n = 24e7;  // or e8
  const double h = (b-a)/n;
  const double ref = 0.73864299803689018;
  const long tseed = 10;
  double res = 0;
  double t0, t1;


    // Initialize number of processes to number of available processors
    long num_processors = sysconf(_SC_NPROCESSORS_ONLN);
    int num_processes = (num_processors > 0) ? (int)num_processors : 1;

  //check for num of args
   if (argc > 3) {
        fprintf(stderr, "Usage: %s [num_processes] [num_samples]\n", argv[0]);
        exit(1);
    }

    if (argc >= 2) {
        char *endptr;
        errno = 0;
        long temp = strtol(argv[1], &endptr, 10);
        if (errno != 0 || *endptr != '\0' || temp <= 0) {
            fprintf(stderr, "Invalid number of processes\n");
            exit(1);
        }
        num_processes = (int)temp;
    }

    if (argc == 3) {
        char *endptr;
        errno = 0;
        n = strtoul(argv[2], &endptr, 10);
        if (errno != 0 || *endptr != '\0' || n == 0) {
            fprintf(stderr, "Invalid number of samples\n");
            exit(1);
        }
    }
  
  // Create shared memory for SharedData structure
    SharedData *shared_data = mmap(NULL, sizeof(SharedData), 
                                   PROT_READ | PROT_WRITE, 
                                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap SharedData");
        exit(1);
    }
  
  // Allocate memory for partial_sums
    shared_data->partial_sums = mmap(NULL, num_processes * sizeof(double), 
                                     PROT_READ | PROT_WRITE, 
                                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_data->partial_sums == MAP_FAILED) {
        perror("mmap partial_sums");
        munmap(shared_data, sizeof(SharedData));
        exit(1);
    }

    shared_data->num_processes = num_processes;

    t0 = get_wtime();

      // Spawn child processes
    for (int i = 0; i < num_processes; i++) {
        pid_t pid = fork();
        if (pid == 0) {  // Child process
            unsigned long samples_per_process = n / num_processes;
            unsigned long start = i * samples_per_process;
            unsigned long end = (i == num_processes - 1) ? n : (i + 1) * samples_per_process;
            
            // Initialize random seed for each child process
            srand(time(NULL) ^ (getpid()<<16));
            
            double partial_sum = 0;
            for (unsigned long j = start; j < end; j++) {
                double xi = (double)rand() / RAND_MAX;
                partial_sum += f(xi);
            }
            shared_data->partial_sums[i] = partial_sum;
            exit(0);
        } else if (pid < 0) {
            perror("fork");
            exit(1);
        }
    }

    // Wait for all child processes to complete
    for (int i = 0; i < num_processes; i++) {
        wait(NULL);
    }

    // Sum up partial results
    for (int i = 0; i < shared_data->num_processes; i++) {
        res += shared_data->partial_sums[i];
    }
    res *= h;

    t1 = get_wtime();

    printf("Result=%.16f Error=%e Rel.Error=%e Time=%lf seconds\n",
           res, fabs(res-ref), fabs(res-ref)/ref, t1-t0);

    // Clean up shared memory
    if (munmap(shared_data->partial_sums, num_processes * sizeof(double)) == -1) {
        perror("munmap partial_sums");
    }
    if (munmap(shared_data, sizeof(SharedData)) == -1) {
        perror("munmap shared_data");
    }

  return 0;
}
