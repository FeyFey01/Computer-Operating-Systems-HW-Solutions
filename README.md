# BLG 312E - Operating Systems Homeworks

[cite_start]This repository contains implementations for two major projects in the BLG 312E Operating Systems course at Istanbul Technical University. [cite: 144]

---

## Homework 1: Shell Executor (itush)
[cite_start]This homework is about process management and inter-process communication. [cite: 144] The solution involves:
* [cite_start]**Process Control**: Implementing `fork()` and `execvp()` to handle command execution. [cite: 287, 292]
* [cite_start]**Pipelining**: Using `pipe()` to facilitate communication between child processes. [cite: 263, 286]
* [cite_start]**I/O Redirection**: Manipulating file descriptors with `dup2()` to handle input (`<`) and output (`>`) redirection. [cite: 268, 291]
* [cite_start]**State Management**: Capturing exit statuses via `waitpid()` and ensuring zero memory leaks via Valgrind. [cite: 261, 262, 293]



## Homework 2: Thread & Synchronization Library
This homework is about concurrency and thread scheduling. The solution involves:
* **Thread Library**: Building a threading system from scratch, replacing native libraries.
* **Synchronization**: Implementing **Semaphores** and **Mutexes** to manage critical sections.
* **Priority Scheduling**: Integrating priority queueing for thread synchronization, modeled after the Pintos OS.
* **Dockerized Environment**: Running multi-container setups via Docker Compose.
