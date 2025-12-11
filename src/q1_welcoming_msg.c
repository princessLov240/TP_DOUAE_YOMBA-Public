#include <unistd.h>
#include <string.h>

#define WELCOME_MSG "Welcome to ENSEA Shell.\nTo quit, type 'exit'.\n"
#define PROMPT "enseash % "
#define BUFFER_SIZE 128

int main() {
    char buffer[BUFFER_SIZE];

    // Write the welcoming message to standard output
    write(STDOUT_FILENO, WELCOME_MSG, strlen(WELCOME_MSG));
    
    // Write the prompt to standard output
    write(STDOUT_FILENO, PROMPT, strlen(PROMPT));
    
    return 0;

}