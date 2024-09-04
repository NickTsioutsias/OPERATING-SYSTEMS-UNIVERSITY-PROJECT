#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

double get_wtime(void) {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + (double)t.tv_usec * 1.0e-6;
}

inline double f(double x) {
    return sin(cos(x));
}

int main(int argc, char *argv[]) {
    double a = 0.0;
    double b = 1.0;
    unsigned long n = 24e7;
    const double h = (b-a)/n;
    const double ref = 0.73864299803689018;
    int num_processes = 1;
    double res = 0;
    double t0, t1;

    // Get the number of available processors
    long num_processors = sysconf(_SC_NPROCESSORS_ONLN);
    //error msg that does not occur from the user interaction
    if (num_processors < 1) {
        fprintf(stderr, "Error: Could not determine the number of processors.\n");
        exit(1);
    }
    //error print if no argument is given
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <num_processes> [num_iterations]\n", argv[0]);
        exit(1);
    }
    //check if number of processors/processes are not more than available
    num_processes = atoi(argv[1]);
    if (num_processes < 1 || num_processes > num_processors) {
        fprintf(stderr, "Number of processes must be between 1 and %ld\n", num_processors);
        exit(1);
    }
    // number of iterations, if user chooses to not use the default
    if (argc == 3) {
        n = atol(argv[2]);
    }
    // Create shared memory
    double *shared_results = mmap(NULL, num_processors * sizeof(double), 
                                  PROT_READ | PROT_WRITE, 
                                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_results == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    //get time before execution
    t0 = get_wtime();
    // Fork processes
    for (int i = 0; i < num_processes; i++) {
        // create child process
        pid_t pid = fork();
        //check if fork failed
        if (pid < 0) {
            perror("fork");
            exit(1);
        // Child process
        } else if (pid == 0) {  
            unsigned long start = i * (n / num_processes); 
            //calculates the starting point for each process's part of work 
            //n/num_processes: gives the number of iterations each process should handle
            unsigned long end = (i == num_processes - 1) ? n : (i + 1) * (n / num_processes);
            //If this is the last process (i == num_processes - 1), the end point is n = # iterations
            //else, it's the start of the next process's (i + 1) * (n / num_processes)
            double partial_sum = 0.0;

            // Initialize random number generator with a different seed for each process
            srand48(time(NULL) ^ (getpid() << 16));

            for (unsigned long j = start; j < end; j++) {
                double xi = drand48(); // generate random number [0,1] for sampling point
                partial_sum += f(xi); // sum of that process
            }
            //store partial sum of process in the table
            shared_results[i] = partial_sum;
            exit(0);
        }
    }
    // Wait for all child processes to finish
    for (int i = 0; i < num_processes; i++) {
        wait(NULL);
    }
    // Sum up the results
    for (int i = 0; i < num_processes; i++) {
        res += shared_results[i];
    }
    //   
    res *= h;
    //get time after execution
    t1 = get_wtime();
    // print same as sequential
    printf("Result=%.16f Error=%e Rel.Error=%e Time=%lf seconds\n",
           res, fabs(res-ref), fabs(res-ref)/ref, t1-t0);
    // Clean up shared memory
    munmap(shared_results, num_processors * sizeof(double));

    return 0;
}