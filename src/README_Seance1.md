# ENSEA Shell - Part 1: Basic Features

## Introduction
This project implements a mini-shell named `enseash`. This document describes the evolution of the shell through the first four questions.

---

## Question 1: Displaying the prompt and welcome message
**Goal**: Create the basic structure of the shell with a simple prompt.

### System concepts used
- **System calls**: `write()` for output
- **File descriptors**: `STDOUT_FILENO` for standard output
- **Strings**: manipulation with `strlen()`

### Implementation logic
The shell displays a welcome message and a simple prompt (`enseash % `). At this stage, the program ends immediately after displaying the prompt, without reading user input.

---

## Question 2: REPL loop and basic command execution
**Goal**: Implement a Read-Eval-Print-Loop to execute simple commands.

### System concepts used
- **Processes**: `fork()` to create a new process
- **Execution**: `execlp()` to replace the process image
- **Synchronization**: `wait()` to wait for the child process
- **Input/Output**: `read()` to read from standard input
- **Error handling**: using `perror()` and `write()` to report errors

### Implementation logic
1. **Infinite loop**: the shell remains active until interrupted.
2. **Command reading**: `read()` captures user input.
3. **Child process execution**: 
   - `fork()` creates a child process.
   - In the child: `execlp()` executes the command.
   - In the parent: `wait()` waits for the child to finish.
4. **Error handling**: 
   - If `read()` fails, print an error message and exit.
   - If `fork()` fails, use `perror()` to report the failure.
   - If `execlp()` fails, the child prints "Command not found".

---

## Question 3: Handling shell exit
**Goal**: Allow the user to exit the shell cleanly.

### System concepts used
- **EOF signal**: detection of `Ctrl+D` (`bytes_read == 0`)
- **String comparison**: `strncmp()` to identify "exit"
- **Proper termination**: `return` with exit code 0

### Implementation logic
1. **"exit" command**: exact comparison with user input.
2. **Ctrl+D**: detect end-of-file on standard input.
3. **Exit message**: display "Bye bye..." before terminating.
4. **Error management**: ensure the shell handles read failures without crashing.

---

## Question 4: Displaying exit code / signal in the prompt
**Goal**: Show the status of the last executed command in the prompt.

### System concepts used
- **Process status**: `waitpid()` to get the child’s status
- **Macros for analysis**: 
  - `WIFEXITED()` / `WEXITSTATUS()` for normal exits
  - `WIFSIGNALED()` / `WTERMSIG()` for signal terminations
- **String manipulation**: dynamic prompt construction
- **Error handling**: reporting fork or execution errors

### Implementation logic
1. **Status retrieval**: `waitpid()` stores the status in `status`.
2. **Status analysis**:
   - If `WIFEXITED(status)`: command ended normally → exit code.
   - If `WIFSIGNALED(status)`: command terminated by signal → signal number.
3. **Prompt construction**:
   - `int_to_str()`: converts integer to string for display.
   - `display_prompt()`: assembles the prompt with optional status.
   - Format: `enseash [exit:X] % ` or `enseash [sign:Y] % `.
4. **Persistent state**: variables `last_exit_code` and `last_signal` carry over between iterations.
5. **Error management**: fork failures are reported via `perror()`; command execution failures are reported in the child.

---

## General architecture
The shell follows a modular architecture:
1. **Display layer**: manages the prompt and messages.
2. **Execution layer**: `fork()` / `exec()` / `wait()` for commands.
3. **State layer**: tracks the status of previous commands.
4. **Error layer**: handles and reports failures in read, fork, or exec.
5. **Main loop**: orchestrates the REPL cycle.

---

