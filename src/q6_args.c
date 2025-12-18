#define _POSIX_C_SOURCE 199309L // For clock_gettime

#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "enseash.h"


// Convert integer to string (base 10)
void int_to_str(int value, char *str) {
    char temp[12];
    int i = 0;

    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    while (value > 0) {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }

    int j = 0;
    while (i > 0) {
        str[j++] = temp[--i];
    }
    str[j] = '\0';
}

// Build and display the prompt with status information
void display_prompt(int exit_code, int signal_num, long duration_ms) {
    char prompt[PROMPT_BUFFER_SIZE];
    char num_str[12];
    
    strcpy(prompt, PROMPT_BASE);
  // If last command exited with a code or signal, show it  
    if (signal_num != -1) {
        strcat(prompt, " [sign:");
        int_to_str(signal_num, num_str);
        strcat(prompt, num_str);
    } else if (exit_code != -1) {
        strcat(prompt, " [exit:");
        int_to_str(exit_code, num_str);
        strcat(prompt, num_str);
    }

    // Append duration information
    if (duration_ms >= 0) {
        strcat(prompt, "|");
        int_to_str(duration_ms, num_str);
        strcat(prompt, num_str);
        strcat(prompt, "ms]");
    }
    
    strcat(prompt, PROMPT_END);
    write(STDOUT_FILENO, prompt, strlen(prompt));
}

//function to parse a command into arguments for execvp   '
void parse_command( char *command, char *argv[]) {
    int i = 0;
    char *token = strtok(command, " ");
    while (token != NULL && i < BUFFER_SIZE - 1) {
        argv[i++] = token;
        token = strtok(NULL, " ");}
    argv[i] = NULL; // Null-terminate the arguments array
}

// Execute a command in a child process
void execute_command(const char *command, int *exit_code, int *signal_num, long *duration_ms) {
    struct timespec start, end; //time measurement
    char cmd_copy[BUFFER_SIZE]; // Copy of command to parse
    strcpy(cmd_copy, command);

    clock_gettime(CLOCK_MONOTONIC, &start);
    pid_t pid = fork();
    int status;
    
    if (pid < 0) {
        perror("Fork failed");  
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        //parse the command into arguments for execvp
        char *args[BUFFER_SIZE]; 
        parse_command(cmd_copy, args);
        execvp(args[0], args);
        write(STDERR_FILENO, CMD_NOT_FOUND, strlen(CMD_NOT_FOUND));
        _exit(EXIT_FAILURE);
    } else {
        waitpid(pid, &status, 0); // Parent waits for child to finish

        clock_gettime(CLOCK_MONOTONIC, &end);
        // Calculate duration in milliseconds
        *duration_ms = (end.tv_sec - start.tv_sec) * 1000 
        + (end.tv_nsec - start.tv_nsec) / 1000000;
        
        if (WIFEXITED(status)) {
            // Command exited normally
            *exit_code = WEXITSTATUS(status);
            *signal_num = -1;
        } else if (WIFSIGNALED(status)) {
            // Command terminated by signal
            *signal_num = WTERMSIG(status);
            *exit_code = -1;
        }
    }
}




int main() {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    // Variables to store last command's exit code and signal
    int last_exit_code = -1; 
    int last_signal = -1;
    long last_duration = -1;

    write(STDOUT_FILENO, WELCOME_MSG, strlen(WELCOME_MSG));
    
    while (1) {
        // Display prompt with optional exit status or signal
        display_prompt(last_exit_code, last_signal, last_duration);
         // Read user input
        bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
        if (bytes_read < 0) {
            write(STDERR_FILENO, READ_ERROR_MSG, strlen(READ_ERROR_MSG));
            exit(EXIT_FAILURE);
        }
        
        // Handle Ctrl+D (EOF)
        if (bytes_read == 0) {
            write(STDOUT_FILENO, EXIT_MSG, strlen(EXIT_MSG));
            return 0;
        }
        
        // Remove newline at the end of input
        if (buffer[bytes_read - 1] == '\n') {
            buffer[bytes_read - 1] = '\0';
        } else {
            buffer[bytes_read] = '\0';
        }
        // Handle "exit" command
        if (strncmp(buffer, EXIT_CMD, strlen(EXIT_CMD)) == 0 && 
            buffer[strlen(EXIT_CMD)] == '\0') {
            write(STDOUT_FILENO, EXIT_MSG, strlen(EXIT_MSG));
            return 0;
        }
        // Execute the command and update last exit code and signal
        execute_command(buffer, &last_exit_code, &last_signal, &last_duration);
    }
    
    return 0;
}