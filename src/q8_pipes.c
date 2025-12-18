#define _POSIX_C_SOURCE 199309L   // For clock_gettime

#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include "enseash.h"

/* ============================================================
   Convert integer to string (base 10)
   Used to display exit codes, signals and execution time
   ============================================================ */
void int_to_str(int value, char *str) {
    char tmp[12];
    int i = 0;

    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    while (value > 0) {
        tmp[i++] = '0' + (value % 10);
        value /= 10;
    }

    int j = 0;
    while (i > 0) {
        str[j++] = tmp[--i];
    }
    str[j] = '\0';
}

/* ============================================================
   Display ENSEASH prompt with execution status
   Shows exit code or signal and execution time
   ============================================================ */
void display_prompt(int exit_code, int signal_num, long duration_ms) {
    char prompt[PROMPT_BUFFER_SIZE];
    char num[12];

    strcpy(prompt, PROMPT_BASE);

    /* Display exit code or signal information */
    if (signal_num != -1) {
        strcat(prompt, " [sign:");
        int_to_str(signal_num, num);
        strcat(prompt, num);
    } else if (exit_code != -1) {
        strcat(prompt, " [exit:");
        int_to_str(exit_code, num);
        strcat(prompt, num);
    }

    /* Display execution time */
    if (duration_ms >= 0) {
        strcat(prompt, "|");
        int_to_str(duration_ms, num);
        strcat(prompt, num);
        strcat(prompt, "ms]");
    }

    strcat(prompt, PROMPT_END);
    write(STDOUT_FILENO, prompt, strlen(prompt));
}

/* ============================================================
   Parse a command string into argv[] for execvp()
   ============================================================ */
void parse_command(char *command, char *argv[]) {
    int i = 0;
    char *token = strtok(command, " ");

    while (token != NULL && i < BUFFER_SIZE - 1) {
        argv[i++] = token;
        token = strtok(NULL, " ");
    }
    argv[i] = NULL;
}

/* ============================================================
   Handle input (<) and output (>) redirections
   Must be called inside the child process
   ============================================================ */
void handle_redirections(char *argv[]) {
    for (int i = 0; argv[i] != NULL; i++) {

        /* Input redirection */
        if (strcmp(argv[i], "<") == 0) {
            int fd = open(argv[i + 1], O_RDONLY);
            if (fd < 0) {
                perror("Input redirection failed");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);

            /* Remove "< file" from argv */
            for (int j = i; argv[j] != NULL; j++) {
                argv[j] = argv[j + 2];
            }
            i--;
        }

        /* Output redirection */
        if (strcmp(argv[i], ">") == 0) {
            int fd = open(argv[i + 1],
                          O_WRONLY | O_CREAT | O_TRUNC,
                          0644);
            if (fd < 0) {
                perror("Output redirection failed");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);

            /* Remove "> file" from argv */
            for (int j = i; argv[j] != NULL; j++) {
                argv[j] = argv[j + 2];
            }
            i--;
        }
    }
}

/* ============================================================
   Execute a command
   - Supports simple commands
   - Supports redirections < >
   - Supports one pipe |
   ============================================================ */
void execute_command(
    char *command,
    int *exit_code,
    int *signal_num,
    long *time_ms
) {
    struct timespec start, end;
    int status;

    clock_gettime(CLOCK_MONOTONIC, &start);

    /* Search for pipe symbol */
    char *pipe_pos = strchr(command, '|');

    /* ===================== PIPE CASE ===================== */
    if (pipe_pos != NULL) {
        int fd[2];
        pipe(fd);

        /* Split the command into two parts */
        *pipe_pos = '\0';
        char *cmd1 = command;
        char *cmd2 = pipe_pos + 1;

        /* First child: writes to pipe */
        if (fork() == 0) {
            dup2(fd[1], STDOUT_FILENO);
            close(fd[0]);
            close(fd[1]);

            char *argv1[BUFFER_SIZE];
            parse_command(cmd1, argv1);
            handle_redirections(argv1);
            execvp(argv1[0], argv1);
            _exit(EXIT_FAILURE);
        }

        /* Second child: reads from pipe */
        if (fork() == 0) {
            dup2(fd[0], STDIN_FILENO);
            close(fd[1]);
            close(fd[0]);

            char *argv2[BUFFER_SIZE];
            parse_command(cmd2, argv2);
            handle_redirections(argv2);
            execvp(argv2[0], argv2);
            _exit(EXIT_FAILURE);
        }

        close(fd[0]);
        close(fd[1]);
        wait(NULL);
        wait(&status);
    }

    /* ===================== NO PIPE ===================== */
    else {
        pid_t pid = fork();
        if (pid == 0) {
            char *argv[BUFFER_SIZE];
            parse_command(command, argv);
            handle_redirections(argv);
            execvp(argv[0], argv);
            _exit(EXIT_FAILURE);
        } else {
            waitpid(pid, &status, 0);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    /* Compute execution time */
    *time_ms =
        (end.tv_sec - start.tv_sec) * 1000 +
        (end.tv_nsec - start.tv_nsec) / 1000000;

    /* Retrieve exit status */
    if (WIFEXITED(status)) {
        *exit_code = WEXITSTATUS(status);
        *signal_num = -1;
    } else if (WIFSIGNALED(status)) {
        *signal_num = WTERMSIG(status);
        *exit_code = -1;
    }
}




int main() {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    /* Store last command's exit code, signal, and execution time */
    int last_exit_code = -1;
    int last_signal = -1;
    long last_duration = -1;

    /* Display welcome message */
    write(STDOUT_FILENO, WELCOME_MSG, strlen(WELCOME_MSG));

    while (1) {
        /* Display prompt with exit status, signal, and execution time */
        display_prompt(last_exit_code, last_signal, last_duration);

        /* Read user input from STDIN */
        bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
        if (bytes_read < 0) {
            write(STDERR_FILENO, READ_ERROR_MSG, strlen(READ_ERROR_MSG));
            exit(EXIT_FAILURE);
        }

        /* Handle Ctrl+D (EOF) */
        if (bytes_read == 0) {
            write(STDOUT_FILENO, EXIT_MSG, strlen(EXIT_MSG));
            return 0;
        }

        /* Remove newline character at the end of input */
        if (buffer[bytes_read - 1] == '\n') {
            buffer[bytes_read - 1] = '\0';
        } else {
            buffer[bytes_read] = '\0';
        }

        /* Handle "exit" command */
        if (strncmp(buffer, EXIT_CMD, strlen(EXIT_CMD)) == 0 &&
            buffer[strlen(EXIT_CMD)] == '\0') {
            write(STDOUT_FILENO, EXIT_MSG, strlen(EXIT_MSG));
            return 0;
        }

        /* Execute the command and update last exit code, signal, and time */
        execute_command(buffer, &last_exit_code, &last_signal, &last_duration);
    }

    return 0;
}
