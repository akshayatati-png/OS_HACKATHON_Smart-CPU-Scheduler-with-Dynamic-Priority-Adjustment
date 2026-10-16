
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_PROCESSES 100

typedef struct {
    int id;
    int arrival;
    int burst;
    int remaining;
    int base_priority;
    int completion;
    int first_start;
    int queued;
} Process;

typedef struct {
    int pid;       /* -1 means CPU idle */
    int start;
    int end;
} Segment;

Process p[MAX_PROCESSES];
Segment gantt[100000];
int process_count;
int segment_count = 0;
int current_time = 0;

enum {
    FCFS = 1,
    SJF,
    PRIORITY,
    ROUND_ROBIN,
    DYNAMIC_AGING
};

void add_segment(int pid, int start, int end)
{
    if (end <= start)
        return;

    if (segment_count >= (int)(sizeof(gantt) / sizeof(gantt[0]))) {
        printf("Execution history limit reached.\n");
        exit(EXIT_FAILURE);
    }

    gantt[segment_count].pid = pid;
    gantt[segment_count].start = start;
    gantt[segment_count].end = end;
    segment_count++;
}

int all_completed(void)
{
    for (int i = 0; i < process_count; i++) {
        if (p[i].remaining > 0)
            return 0;
    }
    return 1;
}

int next_arrival_time(void)
{
    int next = INT_MAX;

    for (int i = 0; i < process_count; i++) {
        if (p[i].remaining > 0 && p[i].arrival < next)
            next = p[i].arrival;
    }

    return next;
}

int ready_process_exists(void)
{
    for (int i = 0; i < process_count; i++) {
        if (p[i].remaining > 0 && p[i].arrival <= current_time)
            return 1;
    }
    return 0;
}

int waiting_time(int i)
{
    return current_time - p[i].arrival
           - (p[i].burst - p[i].remaining);
}

int effective_priority(int i, int aging_interval)
{
    int waiting = waiting_time(i);
    int improvement = waiting / aging_interval;
    int effective = p[i].base_priority - improvement;

    return effective < 0 ? 0 : effective;
}

void advance_if_idle(void)
{
    if (ready_process_exists())
        return;

    int next = next_arrival_time();

    if (next != INT_MAX && next > current_time) {
        add_segment(-1, current_time, next);
        current_time = next;
    }
}

void execute_process(int i, int duration)
{
    int start = current_time;

    if (p[i].first_start == -1)
        p[i].first_start = current_time;

    if (duration > p[i].remaining)
        duration = p[i].remaining;

    add_segment(p[i].id, start, start + duration);

    current_time += duration;
    p[i].remaining -= duration;

    if (p[i].remaining == 0)
        p[i].completion = current_time;
}

/* FCFS: run the earliest-arriving ready process to completion. */
void run_fcfs(void)
{
    while (!all_completed()) {
        advance_if_idle();

        int selected = -1;

        for (int i = 0; i < process_count; i++) {
            if (p[i].remaining <= 0 || p[i].arrival > current_time)
                continue;

            if (selected == -1 ||
                p[i].arrival < p[selected].arrival ||
                (p[i].arrival == p[selected].arrival &&
                 p[i].id < p[selected].id)) {
                selected = i;
            }
        }

        if (selected != -1)
            execute_process(selected, p[selected].remaining);
    }
}

/* SJF: choose the shortest burst among ready processes. */
void run_sjf(void)
{
    while (!all_completed()) {
        advance_if_idle();

        int selected = -1;

        for (int i = 0; i < process_count; i++) {
            if (p[i].remaining <= 0 || p[i].arrival > current_time)
                continue;

            if (selected == -1 ||
                p[i].burst < p[selected].burst ||
                (p[i].burst == p[selected].burst &&
                 p[i].arrival < p[selected].arrival)) {
                selected = i;
            }
        }

        if (selected != -1)
            execute_process(selected, p[selected].remaining);
    }
}

/* Priority: smaller base-priority value means higher priority. */
void run_priority(void)
{
    while (!all_completed()) {
        advance_if_idle();

        int selected = -1;

        for (int i = 0; i < process_count; i++) {
            if (p[i].remaining <= 0 || p[i].arrival > current_time)
                continue;

            if (selected == -1 ||
                p[i].base_priority < p[selected].base_priority ||
                (p[i].base_priority == p[selected].base_priority &&
                 p[i].arrival < p[selected].arrival)) {
                selected = i;
            }
        }

        if (selected != -1)
            execute_process(selected, p[selected].remaining);
    }
}

void enqueue_arrivals(int queue[], int *tail, int *count)
{
    for (int i = 0; i < process_count; i++) {
        if (p[i].remaining > 0 &&
            p[i].arrival <= current_time &&
            !p[i].queued) {

            queue[*tail] = i;
            *tail = (*tail + 1) % MAX_PROCESSES;
            (*count)++;
            p[i].queued = 1;
        }
    }
}

/* Round Robin: each ready process gets one time quantum. */
void run_round_robin(int quantum)
{
    int queue[MAX_PROCESSES];
    int head = 0, tail = 0, count = 0;

    while (!all_completed()) {
        enqueue_arrivals(queue, &tail, &count);

        if (count == 0) {
            advance_if_idle();
            enqueue_arrivals(queue, &tail, &count);
            continue;
        }

        int i = queue[head];
        head = (head + 1) % MAX_PROCESSES;
        count--;

        /* Keep this process marked queued while it executes,
           preventing it from being added twice. */
        int duration = p[i].remaining < quantum
                       ? p[i].remaining : quantum;

        execute_process(i, duration);

        /* New arrivals enter the queue before the running
           process is placed at the end again. */
        enqueue_arrivals(queue, &tail, &count);

        p[i].queued = 0;

        if (p[i].remaining > 0) {
            queue[tail] = i;
            tail = (tail + 1) % MAX_PROCESSES;
            count++;
            p[i].queued = 1;
        }
    }
}

/* Dynamic priority with aging and time-sliced execution. */
void run_dynamic_aging(int quantum, int aging_interval)
{
    while (!all_completed()) {
        advance_if_idle();

        int selected = -1;
        int best_priority = INT_MAX;
        int longest_wait = -1;

        for (int i = 0; i < process_count; i++) {
            if (p[i].remaining <= 0 || p[i].arrival > current_time)
                continue;

            int effective = effective_priority(i, aging_interval);
            int waiting = waiting_time(i);

            if (selected == -1 ||
                effective < best_priority ||
                (effective == best_priority && waiting > longest_wait) ||
                (effective == best_priority && waiting == longest_wait &&
                 p[i].arrival < p[selected].arrival)) {

                selected = i;
                best_priority = effective;
                longest_wait = waiting;
            }
        }

        if (selected != -1)
            execute_process(selected, quantum);
    }
}

void display_gantt(void)
{
    printf("\n================ GANTT CHART ================\n");

    for (int i = 0; i < segment_count; i++) {
        if (gantt[i].pid == -1)
            printf("| IDLE ");
        else
            printf("|  P%d  ", gantt[i].pid);
    }

    printf("|\n");

    if (segment_count > 0) {
        printf("%d", gantt[0].start);

        for (int i = 0; i < segment_count; i++)
            printf("\t%d", gantt[i].end);

        printf("\n");
    }
}

void display_results(void)
{
    double total_tat = 0;
    double total_wt = 0;
    double total_rt = 0;

    printf("\n================ RESULTS ====================\n");
    printf("%-8s %-8s %-8s %-8s %-8s %-8s %-8s\n",
           "Process", "Arrival", "Burst", "CT", "TAT", "WT", "RT");

    for (int i = 0; i < process_count; i++) {
        int tat = p[i].completion - p[i].arrival;
        int wt = tat - p[i].burst;
        int rt = p[i].first_start - p[i].arrival;

        total_tat += tat;
        total_wt += wt;
        total_rt += rt;

        printf("P%-7d %-8d %-8d %-8d %-8d %-8d %-8d\n",
               p[i].id, p[i].arrival, p[i].burst,
               p[i].completion, tat, wt, rt);
    }

    printf("\nAverage Turnaround Time : %.2f\n",
           total_tat / process_count);
    printf("Average Waiting Time    : %.2f\n",
           total_wt / process_count);
    printf("Average Response Time   : %.2f\n",
           total_rt / process_count);

    display_gantt();
}

int main(void)
{
    int algorithm;
    int quantum = 0;
    int aging_interval = 0;

    printf("\n");
    printf("===============================================\n");
    printf(" SMART CPU SCHEDULER WITH DYNAMIC PRIORITY\n");
    printf("===============================================\n");

    printf("\nSelect the CPU Scheduling Algorithm:\n");
    printf("1. First Come, First Served (FCFS)\n");
    printf("2. Shortest Job First (SJF)\n");
    printf("3. Priority Scheduling\n");
    printf("4. Round Robin (RR)\n");
    printf("5. Dynamic Priority with Aging (Proposed)\n");

    printf("\nEnter your choice (1-5): ");

    if (scanf("%d", &algorithm) != 1 ||
        algorithm < FCFS || algorithm > DYNAMIC_AGING) {
        printf("Invalid algorithm choice.\n");
        return EXIT_FAILURE;
    }

    printf("\nSelected Algorithm: ");

    switch (algorithm) {
        case FCFS:
            printf("First Come, First Served (FCFS)\n");
            break;
        case SJF:
            printf("Shortest Job First (Non-preemptive SJF)\n");
            break;
        case PRIORITY:
            printf("Priority Scheduling (Non-preemptive)\n");
            break;
        case ROUND_ROBIN:
            printf("Round Robin (RR)\n");
            break;
        case DYNAMIC_AGING:
            printf("Dynamic Priority with Aging\n");
            break;
    }

    if (algorithm == ROUND_ROBIN || algorithm == DYNAMIC_AGING) {
        printf("Enter time quantum: ");

        if (scanf("%d", &quantum) != 1 || quantum <= 0) {
            printf("Time quantum must be positive.\n");
            return EXIT_FAILURE;
        }
    }

    if (algorithm == DYNAMIC_AGING) {
        printf("Enter aging interval: ");

        if (scanf("%d", &aging_interval) != 1 ||
            aging_interval <= 0) {
            printf("Aging interval must be positive.\n");
            return EXIT_FAILURE;
        }
    }

    printf("Enter number of processes (1-%d): ", MAX_PROCESSES);

    if (scanf("%d", &process_count) != 1 ||
        process_count < 1 || process_count > MAX_PROCESSES) {
        printf("Invalid number of processes.\n");
        return EXIT_FAILURE;
    }

    printf("\nEnter process details:\n");
    printf("(Arrival time >= 0, burst time > 0)\n");

    for (int i = 0; i < process_count; i++) {
        p[i].id = i + 1;

        printf("\nProcess P%d\n", p[i].id);

        printf("Arrival time: ");
        if (scanf("%d", &p[i].arrival) != 1 ||
            p[i].arrival < 0) {
            printf("Arrival time must be non-negative.\n");
            return EXIT_FAILURE;
        }

        printf("Burst time: ");
        if (scanf("%d", &p[i].burst) != 1 ||
            p[i].burst <= 0) {
            printf("Burst time must be positive.\n");
            return EXIT_FAILURE;
        }

        printf("Base priority (smaller number = higher priority): ");
        if (scanf("%d", &p[i].base_priority) != 1 ||
            p[i].base_priority < 0) {
            printf("Priority must be non-negative.\n");
            return EXIT_FAILURE;
        }

        p[i].remaining = p[i].burst;
        p[i].completion = 0;
        p[i].first_start = -1;
        p[i].queued = 0;
    }

    printf("\nRunning scheduling simulation...\n");

    switch (algorithm) {
        case FCFS:
            run_fcfs();
            break;
        case SJF:
            run_sjf();
            break;
        case PRIORITY:
            run_priority();
            break;
        case ROUND_ROBIN:
            run_round_robin(quantum);
            break;
        case DYNAMIC_AGING:
            run_dynamic_aging(quantum, aging_interval);
            break;
    }

    display_results();

    printf("\nSimulation completed successfully.\n");

    return 0;
}
