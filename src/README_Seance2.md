# ENSEA Shell - Part 2: Advanced Features

## Introduction
This second session focuses on improving the shell's feedback by measuring execution time and allowing complex commands with arguments.

## Question 5: Execution Time Measurement
**Goal:** Display the time taken by each command in the prompt.
- **System call:** `clock_gettime()` with `CLOCK_MONOTONIC`.
- **Reasoning:** We capture the time before the `fork()` and after the `waitpid()`. The difference is converted into milliseconds.
- **Result:** The prompt now looks like `enseash [exit:0|15ms] %`.

## Question 6: Execution of Complex Commands
**Goal:** Enable commands with arguments (e.g., `ls -l`).
- **Function used:** `strtok()` to split the input string.
- **System call:** `execvp()` instead of `execlp()`.
- **Reasoning:** `execvp` accepts an array of strings (`argv`), allowing us to pass multiple arguments to the executed program.
