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
#define EXIT_MSG "Bye bye...\n"
#define EXIT_CMD "exit"


//Print the shell prompt to standard output
void print_prompt() {
    write(STDOUT_FILENO, PROMPT, strlen(PROMPT));
}


//Execute the command entered by the user
void execute_command(char *cmd) {
    pid_t pid = fork(); // create child process

    if (pid < 0) {
        // Fork failed
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process tries to execute the command
        execlp(cmd, cmd, (char *)NULL);
        // If execlp returns, command was not found
        write(STDERR_FILENO, CMD_NOT_FOUND, strlen(CMD_NOT_FOUND));
        _exit(EXIT_FAILURE);
    } else {
        // Parent process waits for the child to finish
        wait(NULL);
    }
}


//Main shell loop
int main() {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // Print welcoming message
    write(STDOUT_FILENO, WELCOME_MSG, strlen(WELCOME_MSG));

    while (1) {
        print_prompt(); // display prompt
        
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

        // Null-terminate input string
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

        // Execute any other command
        execute_command(buffer);
    }

    return 0;
}
