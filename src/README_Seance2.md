# ENSEA Shell – Part 2: Advanced Execution Features

## Introduction
This document describes the second development session of the `enseash` mini-shell.
During this session, the shell was extended to support execution time measurement,
commands with arguments, input/output redirections, and pipe handling.

---

## Question 5: Measuring command execution time
**Goal**: Measure and display the execution time of each command using `clock_gettime`.

### System concepts used
- **High-resolution timers**: `clock_gettime()` with `CLOCK_MONOTONIC`
- **Time structures**: `struct timespec`
- **Process control**: `fork()`, `waitpid()`
- **Status analysis**:
  - `WIFEXITED()`, `WEXITSTATUS()`
  - `WIFSIGNALED()`, `WTERMSIG()`

### Implementation logic
1. **Start time measurement**:
   - The timestamp is recorded just before calling `fork()` using `clock_gettime`.
2. **Command execution**:
   - A child process executes the command using `execlp()`.
   - The parent process waits for the child to terminate using `waitpid()`.
3. **End time measurement**:
   - The timestamp is recorded immediately after the child finishes.
4. **Duration computation**:
   - The execution time is computed in milliseconds:
     ```
     duration_ms =
       (end.tv_sec - start.tv_sec) * 1000 +
       (end.tv_nsec - start.tv_nsec) / 1000000;
     ```
5. **Prompt display**:
   - The execution time is displayed alongside the exit code or signal:
     ```
     enseash [exit:0|10ms] %
     enseash [sign:9|5ms] %
     ```

---

## Question 6: Executing commands with arguments
**Goal**: Support complex commands containing arguments (e.g. `hostname -i`).

### System concepts used
- **Argument vectors**: `argv[]`
- **Command parsing**: `strtok()`
- **Process execution**: `execvp()`

### Implementation logic
1. **Command parsing**:
   - The input string is split into tokens using spaces as separators.
   - Tokens are stored in an `argv[]` array.
2. **Execution**:
   - `execvp()` is used instead of `execlp()` to support arguments.
3. **Process handling**:
   - The shell still relies on `fork()` and `waitpid()` for execution control.
4. **Compatibility**:
   - Simple commands without arguments continue to work normally.

---

## Question 7: Input and output redirections (`<` and `>`)
**Goal**: Redirect standard input and output to or from files.

### System concepts used
- **File operations**: `open()`, `close()`
- **File descriptor duplication**: `dup2()`
- **Redirection operators**: `<`, `>`
- **File permissions**: `0644`

### Implementation logic
1. **Redirection detection**:
   - The argument list is scanned for `<` and `>` symbols.
2. **Input redirection (`<`)**:
   - The specified file is opened in read-only mode.
   - `dup2()` redirects `STDIN_FILENO` to the file descriptor.
3. **Output redirection (`>`)**:
   - The file is opened (created if necessary, truncated if it exists).
   - `dup2()` redirects `STDOUT_FILENO` to the file descriptor.
4. **Argument cleanup**:
   - Redirection symbols and filenames are removed from `argv[]`.
5. **Execution context**:
   - Redirections are handled in the child process before calling `execvp()`.

Example:
enseash % ls > filelist.txt
enseash % wc -l < filelist.txt

---

## Question 8: Pipe handling (`|`)
**Goal**: Implement inter-process communication using pipes.

### System concepts used
- **Anonymous pipes**: `pipe()`
- **Inter-process communication**
- **Multiple processes**: two child processes
- **File descriptor redirection**: `dup2()`

### Implementation logic
1. **Pipe detection**:
   - The command line is searched for the `|` symbol.
2. **Command splitting**:
   - The input is divided into two commands: `cmd1 | cmd2`.
3. **Pipe creation**:
   - `pipe(fd)` creates two file descriptors:
     - `fd[0]`: read end
     - `fd[1]`: write end
4. **First child process**:
   - Redirects `STDOUT_FILENO` to `fd[1]`.
   - Executes the first command.
5. **Second child process**:
   - Redirects `STDIN_FILENO` to `fd[0]`.
   - Executes the second command.
6. **Parent process**:
   - Closes unused file descriptors.
   - Waits for both children to terminate.
7. **Execution time measurement**:
   - The global execution time includes both commands.

Example:
enseash % ls | wc -l

---

## Conclusion
During this second session, the `enseash` shell was significantly enhanced.
It now supports:
- Execution time measurement
- Commands with arguments
- Input and output redirections
- Pipe-based command chaining

These features bring the shell closer to the behavior of a real UNIX shell
and provide a deeper understanding of process management and file descriptor handling.
