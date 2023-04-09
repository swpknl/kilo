//
// Created by swpknl on 8/4/23.
//
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

struct termios original_termios; // Global variable to cleanup on exit

// prints an error message and exits the program.
void die(const char *s)
{
    // perror() looks at the global errno variable and prints a descriptive error message for it
    perror(s);
    exit(1);
}

void disableRawMode()
{
    // Reset the terminal to its original values before exiting
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) == -1)
    {
        die("tcsetattr");
    }
}

// Disable canonical/cooked mode where keyboard input is sent to the program only when the user presses Enter; and enable the raw mode where we process each
// input as it comes in
void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1) // Read terminal attributes via tcgetattr
        die("tcgetattr");
    atexit(disableRawMode); // Disable raw mode when our program exits
    struct termios raw = original_termios;
    // IXON: Disable Ctrl+S and Ctrl+Q
    // ICRNL: Disable carriage return(CR) and new line(NL)
    // BRKINT: Sets break condition, like Ctrl+C
    // INPCK: Enables parity checking
    // ISTRIP: Strips 8th bit of each byte
    raw.c_iflag = ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    // Disable output processing
    raw.c_oflag = ~(OPOST);
    raw.c_cflag |= (CS8);
    // VMIN: The VMIN value sets the minimum number of bytes of input needed before read() can return.
    // VTIME: The VTIME value sets the maximum amount of time to wait before read() returns.
    // c_cc: Stands for “control characters”, an array of bytes that control various terminal settings.
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    // Echo flag causes the keyboard input to be flushed to the terminal. We flip the bits and disable the same.
    // Disabling ICANON flag allows input to be read byte by byte and not line by line as in canonical mode
    // ISIG flag disables processing Ctrl+C which sends SIGINT to terminate and Ctrl+Z which sends SIGSTP which causes the process to be suspended
    // IEXTEN flag disables Ctrl+V
    // c_lflag stands for local flags, place to dump miscallaneous output
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) // Update terminal attributes
        die("tcsetattr");
}

int main()
{
    enableRawMode();
    char c;
    while (1)
    {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");
        if (iscntrl(c)) // Check if the input char is a control character. Control characters are nonprintable characters
        // that we don’t want to print to the screen
        {
            printf("%d\r\n", c); // Print the character as decimal
        }
        else
        {
            printf("%d ('%c')\r\n", c, c); // Print the char as number as well as a character
        }
        if (c == 'q')
            break;
    }
    return 0;
}