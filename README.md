
# Smart CPU Scheduler with Dynamic Priority Adjustment

## 1. Project Overview

The Smart CPU Scheduler is a CPU scheduling simulator developed using C.
It uses priority scheduling with dynamic priority adjustment and aging
to improve process fairness and reduce starvation.

Users can configure the time quantum and aging interval at runtime.
The simulator displays the scheduling sequence, Gantt chart, and
performance metrics for all processes.

## 2. Features

- Dynamic priority adjustment using aging.
- User-configurable time quantum.
- User-configurable aging interval.
- Process arrival time, burst time, and priority inputs.
- Round Robin execution time slices.
- CPU scheduling Gantt chart.
- Waiting time, turnaround time, and response time calculations.
- Input validation for process settings.
- Terminal-based interface.

## 3. Technologies Used

- Programming Language: C
- Compiler: GCC
- Development Environment: Visual Studio Code
- Version Control: Git and GitHub

## 4. Project Structure

```text
Smart-CPU-Scheduler/
├── main.c
├── README.md
└── .gitignore
```

## 5. Requirements

- Windows, Linux, or another supported operating system.
- GCC C compiler.
- Terminal or command prompt.
- Git (for version control and GitHub upload).

## 6. Setup

Install GCC and ensure it is available in your terminal.

Verify the installation:

```bash
gcc --version
```

## 7. Compilation

Open the terminal in the project directory and run:

```bash
gcc -std=c11 -Wall -Wextra -o scheduler.exe main.c
```

On Windows, this creates `scheduler.exe`.

For Linux, compile using:

```bash
gcc -std=c11 -Wall -Wextra -o scheduler main.c
```

## 8. Execution

On Windows PowerShell:

```powershell
.\scheduler.exe
```

On Linux:

```bash
./scheduler
```

## 9. How to Use

1. Enter a positive time quantum.
2. Enter a positive aging interval.
3. Enter the number of processes.
4. Enter each process's arrival time, burst time, and base priority.
5. View the scheduling sequence and Gantt chart.
6. Review the performance metrics.

A smaller priority number represents a higher priority.

The aging interval determines how much accumulated waiting time
is needed for a process's effective priority to improve.

## 10. Performance Metrics

- Completion Time (CT): Time when a process finishes.
- Turnaround Time (TAT): Completion time minus arrival time.
- Waiting Time (WT): Turnaround time minus burst time.
- Response Time (RT): First CPU start time minus arrival time.
- Average Waiting Time: Average waiting time of all processes.
- Average Turnaround Time: Average turnaround time of all processes.
- Average Response Time: Average response time of all processes.

## 11. Limitations

- This is a scheduling simulator, not a modification of the OS kernel.
- The number of processes is limited to 20.
- The scheduling algorithm uses simplified priority-aging rules.
- Performance depends on the input workload and scheduler settings.

## 12. Future Enhancements

- Add a graphical user interface.
- Compare multiple CPU scheduling algorithms.
- Visualize priority changes over time.
- Export scheduling results to a file.
- Add interactive performance charts.

## 13. Author

Developed as an Operating Systems hackathon project.

## 14. License

This project is available for educational and demonstration purposes.
