#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>

#define WELCOME_MSG "Welcome to ENSEA Shell.\nTo quit, type 'exit'.\n"
#define PROMPT "enseash % "
#define BUFFER_SIZE 128
#define READ_ERROR_MSG "Read error\n"
#define CMD_NOT_FOUND "Command not found\n"


//Print the shell prompt to stdout
void print_prompt() {
    write(STDOUT_FILENO, PROMPT, strlen(PROMPT));
}

//Execute the command entered by the user
void execute_command(char *cmd) {
    pid_t pid = fork();

    if (pid < 0) {
        // Fork failed
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process: execute the command
        execlp(cmd, cmd, (char *)NULL);
        // If execlp returns, the command was not found
        write(STDERR_FILENO, CMD_NOT_FOUND, strlen(CMD_NOT_FOUND));
        _exit(EXIT_FAILURE);
    } else {
        // Parent waits for child to finish
        wait(NULL);
    }
}


int main() {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // Display welcome message
    write(STDOUT_FILENO, WELCOME_MSG, strlen(WELCOME_MSG));

    while (1) {
        print_prompt();  // Show prompt

        // Read user input
        bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
        if (bytes_read < 0) {
            write(STDERR_FILENO, READ_ERROR_MSG, strlen(READ_ERROR_MSG));
            exit(EXIT_FAILURE);
        }

        // Null-terminate input string
        if (bytes_read > 0 && buffer[bytes_read - 1] == '\n') {
            buffer[bytes_read - 1] = '\0';
        } else {
            buffer[bytes_read] = '\0';
        }

        // Execute the command
        execute_command(buffer);
    }

    return 0;
}
