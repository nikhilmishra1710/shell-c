#include <stdio.h>
#include <string.h>

typedef enum { false, true } bool;

int main()
{
  // Flush after every printf
  bool run = true;

  while (run)
  {
    setbuf(stdout, NULL);

    // Uncomment this block to pass the first stage
    printf("$ ");

    // Wait for user input
    char input[100];
    fgets(input, 100, stdin);

    // Remove trailing newline character
    input[strcspn(input, "\n")] = 0;

    // Print the user input
    printf("%s: command not found\n", input);
  }
  return 0;
}