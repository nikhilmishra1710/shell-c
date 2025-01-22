#include <stdio.h>
#include <string.h>

typedef enum { false, true } bool;

bool compareString(char *str1, char *str2)
{
  if (strlen(str1) != strlen(str2))
  {
    return false;
  }
  else if(strcmp(str1, str2) != 0)
  {
    return false;
  }
  return true;
}

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

    if(compareString(input, "exit 0"))
    {
      run = false;
    }else{
      printf("%s: command not found\n", input);
    }
    // Print the user input
  }

  return 0;
}
