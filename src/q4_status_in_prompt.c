#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>


#define WELCOMING_MSG "Welcome to the ENSEA's shell!\nIf you want to quit, type 'exit'.\n"
#define PROMPT "enseash"
#define PROMT_END "%"
#define BUFFER_SIZE 128
#define CMD_NOT_FOUND "Command not found\n"
#define EXIT_MSG "Bye bye...\n"
#define EXIT_STATUS "[exit:"
#define EXIT_SIGNAL "[sign:"
#define EXIT_END "]"

// Convertit un entier en chaîne (base 10)
void int_to_str(int value, char *str) {
    char temp[12];
    int i = 0;

    // Gérer le cas 0
    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    // Convertir les chiffres
    while (value > 0) {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }

    // Inverser les chiffres
    int j = 0;
    while (i > 0) {
        str[j++] = temp[--i];
    }

    str[j] = '\0';
}
4
int main() {
    char buffer[BUFFER_SIZE], exit_msg[BUFFER_SIZE] ;
    int last_status = -1;
    int last_was_exit = -1;
    int last_signal = -1;

    // Write the welcoming message to standard output
    write(STDOUT_FILENO, WELCOMING_MSG, strlen(WELCOMING_MSG));
    while (1)
    {
        
        if (last_signal != -1)
        {
          
            // If the last command was terminated by a signal
            char num[12];
            int_to_str(last_signal, num);

            strcpy(exit_msg, PROMPT);
            strcat(exit_msg, EXIT_SIGNAL);
            strcat(exit_msg, num);
            strcat(exit_msg, EXIT_END);
            strcat(exit_msg, PROMT_END);
            
            last_signal = 0;
                   
              
           } else if (last_status != -1){
            // If the last command exited with a non-zero status
            char num[12];
            int_to_str(last_status, num);

            strcpy(exit_msg, PROMPT);
            strcat(exit_msg, EXIT_STATUS);
            strcat(exit_msg, num);
            strcat(exit_msg, EXIT_END);
            strcat(exit_msg, PROMT_END);
            last_status = 0;
               
               
           }
        
           else {
            strcpy(exit_msg, PROMPT);
            strcat(exit_msg, PROMT_END);

           
        }
    
        
        // Write the prompt to standard output
        write(STDOUT_FILENO, exit_msg, strlen(exit_msg));
        // Read user input from standard input
        ssize_t size = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);

        
        if (size < 0) {
            // Handle read error
            perror ("Erreur de lecture");
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
        int status;

        if (pid < 0) {
            perror("Fork impossible"); // Fork failed
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            execlp(buffer, buffer, (char *)NULL);
            // If execlp returns, an error occurred
            write(STDOUT_FILENO, CMD_NOT_FOUND, strlen(CMD_NOT_FOUND));
            _exit(127);
        } else {
            // Parent process waits for child to finish
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                // Child exited normally
                 last_status = WEXITSTATUS(status);
                 last_signal = -1;
            } else if (WIFSIGNALED(status)) {
                // Child was terminated by a signal
                 last_signal = WTERMSIG(status);
                 last_status = -1;
        }
    }
       
    }
    return 0;
}





