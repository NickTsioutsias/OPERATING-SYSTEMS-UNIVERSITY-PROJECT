#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <semaphore.h>

#define SEM_NAME "/integral_sem"

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
    const double ref = 0.73864299803689018;
    int num_processes = 1;
    double res = 0;
    double t0, t1;

    // Get the number of available processors
    long num_processors = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_processors < 1) {
        fprintf(stderr, "Error: Could not determine the number of processors.\n");
        exit(1);
    }

    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <num_processes> [num_iterations]\n", argv[0]);
        exit(1);
    }

    num_processes = atoi(argv[1]);
    if (num_processes < 1 || num_processes > num_processors) {
        fprintf(stderr, "Number of processes must be between 1 and %ld\n", num_processors);
        exit(1);
    }

    const double h = (b-a)/n;

    // Create shared memory
    double *shared_results = mmap(NULL, num_processes * sizeof(double), 
                                  PROT_READ | PROT_WRITE, 
                                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_results == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Create semaphore
    sem_t *sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    t0 = get_wtime();

    // Fork processes
    for (int i = 0; i < num_processes; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {  // Child process
            unsigned long iterations = n / num_processes;
            double partial_sum = 0.0;

            // Initialize random number generator with a different seed for each process
            srand48(time(NULL) ^ (getpid() << 16));

            for (unsigned long j = 0; j < iterations; j++) {
                double xi = drand48();
                partial_sum += f(xi);
            }

            // Critical section: update shared memory
            sem_wait(sem);
            shared_results[i] = partial_sum;
            sem_post(sem);

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
    res *= h;

    t1 = get_wtime();

    printf("Result=%.16f Error=%e Rel.Error=%e Time=%lf seconds\n",
           res, fabs(res-ref), fabs(res-ref)/ref, t1-t0);

    // Clean up
    munmap(shared_results, num_processes * sizeof(double));
    sem_close(sem);
    sem_unlink(SEM_NAME);

    return 0;
}