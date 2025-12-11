#include <unistd.h>
#include <string.h>

#define WELCOMING_MSG "Welcome to the ENSEA's shell!\nIf you want to quit, type 'exit'.\n"
#define PROMPT "enseash %"
#define BUFFER_SIZE 128

int main() {
    char buffer[BUFFER_SIZE];

    // Write the welcoming message to standard output
    write(STDOUT_FILENO, WELCOMING_MSG, strlen(WELCOMING_MSG));
    
    // Write the prompt to standard output
    write(STDOUT_FILENO, PROMPT, strlen(PROMPT));
    
    return 0;

}