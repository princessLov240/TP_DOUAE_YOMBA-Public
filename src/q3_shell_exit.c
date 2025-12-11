#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>



#define WELCOMING_MSG "Welcome to the ENSEA's shell!\nIf you want to quit, type 'exit'.\n"
#define PROMPT "enseash %"
#define BUFFER_SIZE 128
#define CMD_NOT_FOUND "Command not found\n"
#define EXIT_MSG "Bye bye...\n"

int main() {
    char buffer[BUFFER_SIZE];

    // Write the welcoming message to standard output
    write(STDOUT_FILENO, WELCOMING_MSG, strlen(WELCOMING_MSG));
    while (1)
    {
        // Write the prompt to standard output
        write(STDOUT_FILENO, PROMPT, strlen(PROMPT));
        // Read user input from standard input
        ssize_t size = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
        if (size < 0) {
            perror("Error while reading");
            exit(EXIT_FAILURE);
    }
 
     // Null-terminate the input string
        if (buffer[size-1] == '\n') {
            buffer[size-1] = '\0';
        }


        if (strncmp(buffer, "exit",4) == 0 || size == 0){
            write(STDOUT_FILENO, EXIT_MSG, strlen(EXIT_MSG));
            return 0;
        }

        __pid_t pid = fork();
        if (pid < 0) {
            return 1; // Fork failed
        } else if (pid == 0) {
            // Child process
            execlp(buffer, buffer-1, (char *)NULL);
            // If execlp returns, an error occurred
            write(STDOUT_FILENO, CMD_NOT_FOUND, strlen(CMD_NOT_FOUND));
            _exit(127);
        } else {
            wait(NULL); // Parent process waits for child to finish
        }
    }
    
    return 0;
}