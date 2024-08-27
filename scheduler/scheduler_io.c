#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>

#define MAX_PROCESSES 100

typedef enum {
    NEW,
    RUNNING,
    STOPPED,
    WAITING_IO,
    READY,
    EXITED
} ProcessState;

typedef struct {
    char name[256];
    pid_t pid;
    ProcessState state;
    struct timeval start_time;
    struct timeval end_time;
} Process;

Process processes[MAX_PROCESSES];
int process_count = 0;
int current_process = -1;

char policy[10];
int quantum_ms = 0;

void read_input_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening input file");
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        strcpy(processes[process_count].name, line);
        processes[process_count].state = NEW;
        process_count++;
    }

    fclose(file);
}

void start_process(int index) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execl(processes[index].name, processes[index].name, NULL);
        perror("execl failed");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        processes[index].pid = pid;
        processes[index].state = RUNNING;
        gettimeofday(&processes[index].start_time, NULL);
        current_process = index;
    } else {
        perror("fork failed");
        exit(1);
    }
}

void handle_child_exit(int sig) {
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG);
    if (pid > 0) {
        for (int i = 0; i < process_count; i++) {
            if (processes[i].pid == pid) {
                processes[i].state = EXITED;
                gettimeofday(&processes[i].end_time, NULL);

                //Get execution time
                double execution_time = 
                    (processes[i].end_time.tv_sec - processes[i].start_time.tv_sec) +
                    (processes[i].end_time.tv_usec - processes[i].start_time.tv_usec) / 1000000.0;
                    printf("Process %s (PID %d) exited. Execution time: %.6f seconds\n",
                           processes[i].name, processes[i].pid, execution_time);
                    break;
            }
        }
    }
}

void handle_io_start(int sig) {
    if (current_process != -1) {
        processes[current_process].state = WAITING_IO;
        printf("Process %s (PID %d) started I/O\n", processes[current_process].name, processes[current_process].pid);
    }
}

void handle_io_complete(int sig) {
    if (current_process != -1) {
        processes[current_process].state = READY;
        printf("Process %s (PID %d) completed I/O\n", processes[current_process].name, processes[current_process].pid);
    }
}

void switch_process(int from, int to) {
    if (from != -1 && processes[from].state == RUNNING) {
        kill(processes[from].pid, SIGSTOP);
        processes[from].state = STOPPED;
    }
    if (to != -1) {
        if (processes[to].state == NEW) {
            start_process(to);
        } else if (processes[to].state == STOPPED || processes[to].state == READY) {
            kill(processes[to].pid, SIGCONT);
            processes[to].state = RUNNING;
        }
        current_process = to;
    }
}

void run_fcfs() {
    for (int i = 0; i < process_count; i++) {
        start_process(i);
        while (processes[i].state != EXITED) {
            pause();
            if (processes[i].state == WAITING_IO) {
                // Wait for I/O to complete
                while (processes[i].state != READY && processes[i].state != EXITED) {
                    pause();
                }
                if (processes[i].state == READY) {
                    kill(processes[i].pid, SIGCONT);
                    processes[i].state = RUNNING;
                }
            }
        }
    }
}

void run_rr() {
    int time_slice = 0;
    int next_process = 0;

    while (1) {
        if (current_process == -1 || processes[current_process].state == EXITED || 
            processes[current_process].state == WAITING_IO) {
            // Find next runnable process
            do {
                next_process = (next_process + 1) % process_count;
                if (next_process == 0) {
                    // Checked all processes
                    int all_exited = 1;
                    for (int i = 0; i < process_count; i++) {
                        if (processes[i].state != EXITED) {
                            all_exited = 0;
                            break;
                        }
                    }
                    if (all_exited) return;
                }
            } while (processes[next_process].state == EXITED || processes[next_process].state == WAITING_IO);

            switch_process(current_process, next_process);
            time_slice = 0;
        }

        usleep(1000); // Sleep for 1ms
        time_slice++;

        if (time_slice >= quantum_ms) {
            next_process = (current_process + 1) % process_count;
            while (processes[next_process].state == EXITED || processes[next_process].state == WAITING_IO) {
                next_process = (next_process + 1) % process_count;
                if (next_process == current_process) break; // No other runnable process
            }
            if (next_process != current_process) {
                switch_process(current_process, next_process);
                time_slice = 0;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <policy> [<quantum>] <input_file>\n", argv[0]);
        exit(1);
    }

    strcpy(policy, argv[1]);
    char *input_file;

    if (strcmp(policy, "RR") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Usage for RR: %s RR <quantum> <input_file>\n", argv[0]);
            exit(1);
        }
        quantum_ms = atoi(argv[2]);
        input_file = argv[3];
    } else if (strcmp(policy, "FCFS") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Usage for FCFS: %s FCFS <input_file>\n", argv[0]);
            exit(1);
        }
        input_file = argv[2];
    } else {
        fprintf(stderr, "Unknown policy: %s\n", policy);
        exit(1);
    }

    read_input_file(input_file);

    signal(SIGCHLD, handle_child_exit);
    signal(SIGUSR1, handle_io_start);
    signal(SIGUSR2, handle_io_complete);

    if (strcmp(policy, "FCFS") == 0) {
        run_fcfs();
    } else if (strcmp(policy, "RR") == 0) {
        run_rr();
    }

    return 0;
}