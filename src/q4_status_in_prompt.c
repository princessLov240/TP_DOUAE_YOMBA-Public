#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>

#define WELCOME_MSG "Welcome to ENSEA Shell.\nTo quit, type 'exit'.\n"
#define PROMPT_BASE "enseash"
#define PROMPT_END " % "
#define BUFFER_SIZE 128
#define READ_ERROR_MSG "Read error\n"
#define PROMPT_BUFFER_SIZE 64
#define CMD_NOT_FOUND "Command not found\n"
#define EXIT_MSG "Bye bye...\n"
#define EXIT_CMD "exit"

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
void display_prompt(int exit_code, int signal_num) {
    char prompt[PROMPT_BUFFER_SIZE];
    char num_str[12];
    
    strcpy(prompt, PROMPT_BASE);
  // If last command exited with a code or signal, show it  
    if (signal_num != -1) {
        strcat(prompt, " [sign:");
        int_to_str(signal_num, num_str);
        strcat(prompt, num_str);
        strcat(prompt, "]");
    } else if (exit_code != -1) {
        strcat(prompt, " [exit:");
        int_to_str(exit_code, num_str);
        strcat(prompt, num_str);
        strcat(prompt, "]");
    }
    
    strcat(prompt, PROMPT_END);
    write(STDOUT_FILENO, prompt, strlen(prompt));
}

// Execute a command in a child process
void execute_command(const char *command, int *exit_code, int *signal_num) {
    pid_t pid = fork();
    int status;
    
    if (pid < 0) {
        perror("Fork failed");  
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        execlp(command, command, (char *)NULL);
        write(STDERR_FILENO, CMD_NOT_FOUND, strlen(CMD_NOT_FOUND));
        _exit(EXIT_FAILURE);
    } else {
        waitpid(pid, &status, 0); // Parent waits for child to finish
        
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

    write(STDOUT_FILENO, WELCOME_MSG, strlen(WELCOME_MSG));
    
    while (1) {
        // Display prompt with optional exit status or signal
        display_prompt(last_exit_code, last_signal);
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
        execute_command(buffer, &last_exit_code, &last_signal);
    }
    
    return 0;
}