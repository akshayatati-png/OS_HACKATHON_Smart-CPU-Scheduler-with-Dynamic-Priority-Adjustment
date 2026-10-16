
#include <stdio.h>

#define MAX_PROCESSES 20
#define MAX_SEGMENTS 10000

typedef struct {
    int pid;
    int arrival;
    int burst;
    int remaining;
    int base_priority;
    int completion;
    int first_start;
    int executed;
} Process;

typedef struct {
    int pid;  /* 0 represents CPU idle */
    int start;
    int end;
} Segment;

Process p[MAX_PROCESSES];
Segment gantt[MAX_SEGMENTS];

int main(void) {
    int n, time = 0, completed = 0, segments = 0;
    int time_quantum, aging_interval;

    double total_wait = 0;
    double total_turnaround = 0;
    double total_response = 0;

    printf("\n");
    printf("==================================================\n");
    printf("       SMART CPU SCHEDULER\n");
    printf("   Dynamic Priority Adjustment with Aging\n");
    printf("==================================================\n");
    printf("Smaller priority number = higher priority\n");

    /* Get scheduler configuration */
    printf("\nEnter Time Quantum (greater than 0): ");
    if (scanf("%d", &time_quantum) != 1 ||
        time_quantum <= 0) {
        printf("Invalid time quantum.\n");
        return 1;
    }

    printf("Enter Aging Interval (greater than 0): ");
    if (scanf("%d", &aging_interval) != 1 ||
        aging_interval <= 0) {
        printf("Invalid aging interval.\n");
        return 1;
    }

    printf("\nTime Quantum   : %d\n", time_quantum);
    printf("Aging Interval : %d\n", aging_interval);

    /* Get processes */
    printf("\nEnter number of processes (1-%d): ",
           MAX_PROCESSES);

    if (scanf("%d", &n) != 1 ||
        n < 1 || n > MAX_PROCESSES) {
        printf("Invalid number of processes.\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        printf("\n--- Process P%d ---\n", i + 1);

        p[i].pid = i + 1;

        printf("Arrival Time: ");
        if (scanf("%d", &p[i].arrival) != 1 ||
            p[i].arrival < 0) {
            printf("Arrival time must be non-negative.\n");
            return 1;
        }

        printf("Burst Time: ");
        if (scanf("%d", &p[i].burst) != 1 ||
            p[i].burst <= 0) {
            printf("Burst time must be positive.\n");
            return 1;
        }

        printf("Base Priority (1-10): ");
        if (scanf("%d", &p[i].base_priority) != 1 ||
            p[i].base_priority < 1 ||
            p[i].base_priority > 10) {
            printf("Priority must be between 1 and 10.\n");
            return 1;
        }

        p[i].remaining = p[i].burst;
        p[i].completion = 0;
        p[i].first_start = -1;
        p[i].executed = 0;
    }

    printf("\n========== SCHEDULING STARTED ==========\n");

    while (completed < n) {
        int selected = -1;
        int best_priority = 100000;
        int longest_wait = -1;

        /* Select the ready process with the best
           dynamically adjusted priority. */
        for (int i = 0; i < n; i++) {
            if (p[i].remaining <= 0 ||
                p[i].arrival > time) {
                continue;
            }

            int waiting = time - p[i].arrival
                        - p[i].executed;

            int effective_priority =
                p[i].base_priority
                - waiting / aging_interval;

            if (effective_priority < 0) {
                effective_priority = 0;
            }

            if (selected == -1 ||
                effective_priority < best_priority ||
                (effective_priority == best_priority &&
                 waiting > longest_wait)) {

                selected = i;
                best_priority = effective_priority;
                longest_wait = waiting;
            }
        }

        /* No process is ready: CPU remains idle. */
        if (selected == -1) {
            if (segments >= MAX_SEGMENTS) {
                printf("Scheduling limit exceeded.\n");
                return 1;
            }

            if (segments > 0 &&
                gantt[segments - 1].pid == 0 &&
                gantt[segments - 1].end == time) {

                gantt[segments - 1].end++;
            } else {
                gantt[segments].pid = 0;
                gantt[segments].start = time;
                gantt[segments].end = time + 1;
                segments++;
            }

            time++;
            continue;
        }

        Process *current = &p[selected];

        if (current->first_start == -1) {
            current->first_start = time;
        }

        /* Run for one time quantum or until completion. */
        int run = current->remaining < time_quantum
                ? current->remaining
                : time_quantum;

        if (segments >= MAX_SEGMENTS) {
            printf("Scheduling limit exceeded.\n");
            return 1;
        }

        gantt[segments].pid = current->pid;
        gantt[segments].start = time;
        gantt[segments].end = time + run;
        segments++;

        printf("Time %d-%d: P%d | Effective Priority: %d\n",
               time, time + run,
               current->pid, best_priority);

        current->remaining -= run;
        current->executed += run;
        time += run;

        if (current->remaining == 0) {
            current->completion = time;
            completed++;

            printf("P%d completed at time %d\n",
                   current->pid, time);
        }
    }

    /* Display Gantt chart */
    printf("\n========== GANTT CHART ==========\n");

    for (int i = 0; i < segments; i++) {
        if (gantt[i].pid == 0) {
            printf("| IDLE ");
        } else {
            printf("|  P%d  ", gantt[i].pid);
        }
    }
    printf("|\n");

    printf("%d", gantt[0].start);

    for (int i = 0; i < segments; i++) {
        printf("%7d", gantt[i].end);
    }
    printf("\n");

    /* Calculate performance metrics */
    printf("\n========== PROCESS RESULTS ==========\n");
    printf("PID\tAT\tBT\tPriority\tCT\tTAT\tWT\tRT\n");

    for (int i = 0; i < n; i++) {
        int turnaround =
            p[i].completion - p[i].arrival;

        int waiting =
            turnaround - p[i].burst;

        int response =
            p[i].first_start - p[i].arrival;

        printf("P%d\t%d\t%d\t%d\t\t%d\t%d\t%d\t%d\n",
               p[i].pid,
               p[i].arrival,
               p[i].burst,
               p[i].base_priority,
               p[i].completion,
               turnaround,
               waiting,
               response);

        total_wait += waiting;
        total_turnaround += turnaround;
        total_response += response;
    }

    printf("\n========== PERFORMANCE SUMMARY ==========\n");
    printf("Average Waiting Time    : %.2f\n",
           total_wait / n);
    printf("Average Turnaround Time : %.2f\n",
           total_turnaround / n);
    printf("Average Response Time   : %.2f\n",
           total_response / n);
    printf("Total Execution Time    : %d\n", time);

    printf("\n==========================================\n");
    printf(" Scheduling completed successfully!\n");
    printf(" Dynamic priority aging was enabled.\n");
    printf("==========================================\n");

    return 0;
}
