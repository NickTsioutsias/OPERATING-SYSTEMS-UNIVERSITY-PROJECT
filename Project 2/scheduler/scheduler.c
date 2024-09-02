#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>

#define MAX_PROCESSES 10

typedef enum {
    NEW,
    RUNNING,
    STOPPED,
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
int quantum = 0;

// Read file and put processes in data structure
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
    pid_t pid;

    // Use a loop to handle multiple child exits in case more than one process has terminated
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < process_count; i++) {
            if (processes[i].pid == pid) {
                processes[i].state = EXITED;
                gettimeofday(&processes[i].end_time, NULL);

                // Calculate execution time
                double execution_time = 
                    (processes[i].end_time.tv_sec - processes[i].start_time.tv_sec) +
                    (processes[i].end_time.tv_usec - processes[i].start_time.tv_usec) / 1000000.0;

                // Print process finished message and execution time
                printf("Process %s (PID %d) exited. Execution time: %.6f seconds\n",
                       processes[i].name, processes[i].pid, execution_time);

                break;
            }
        }
    }
}

void run_fcfs() {
    for (int i = 0; i < process_count; i++) {
        start_process(i);
        while (processes[i].state != EXITED) {
            pause();
        }
    }
}

void switch_process(int from, int to) {
    // Stop the current process if it's not -1
    if (from != -1) {
        if (processes[from].state == RUNNING) {
            printf("Stopping process %d (%s)\n", from, processes[from].name);
            kill(processes[from].pid, SIGSTOP);
            processes[from].state = STOPPED;
        }
    }
    
    // Start or resume the next process if it's not -1
    if (to != -1) {
        if (processes[to].state == NEW) {
            printf("Starting new process %d (%s)\n", to, processes[to].name);
            start_process(to);
        } else if (processes[to].state == STOPPED) {
            printf("Resuming stopped process %d (%s)\n", to, processes[to].name);
            kill(processes[to].pid, SIGCONT);
            processes[to].state = RUNNING;
        }
        
        current_process = to;
    } else {
        current_process = -1;
    }
}


void run_rr() {
    int time_slice = 0;
    int next_process = 0;

    while (1) {
        // Check if all processes have exited
        int all_exited = 1;
        for (int i = 0; i < process_count; i++) {
            if (processes[i].state != EXITED) {
                all_exited = 0;
                break;
            }
        }

        // Exit if all processes have exited
        if (all_exited) {
            printf("All processes have exited. Ending RR.\n");
            break;
        }

        // If the current process is invalid or has exited, find the next process
        if (current_process == -1 || processes[current_process].state == EXITED) {
            // Start from the first process if current_process is -1
            if (current_process == -1) {
                next_process = 0;
            } else {
                // Start from the next process if current_process is not -1
                next_process = (current_process + 1) % process_count;
            }
            
            // Skip over exited processes
            while (processes[next_process].state == EXITED) {
                next_process = (next_process + 1) % process_count;
            }

            // Switch to the next valid process
            printf("Switching from process %d to %d\n", current_process, next_process);
            switch_process(current_process, next_process);
            time_slice = 0;
        }

        // Sleep for 1ms to simulate time passing
        usleep(1000); 
        time_slice++;

        // Check if quantum expired and switch process
        if (time_slice >= quantum) {
            // Move to the next process
            do {
                next_process = (current_process + 1) % process_count;
            } while (processes[next_process].state == EXITED);

            printf("Quantum expired. Switching from process %d to %d\n", current_process, next_process);
            switch_process(current_process, next_process);
            time_slice = 0;
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
        quantum = atoi(argv[2]);
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

    if (strcmp(policy, "FCFS") == 0) {
        run_fcfs();
    } else if (strcmp(policy, "RR") == 0) {
        run_rr();
    }

    return 0;
}