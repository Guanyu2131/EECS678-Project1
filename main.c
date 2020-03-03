#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_LENGTH 1024

int exitQuash(char *cmd)
{
  if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0)
  {
    return 1;
  }

  return 0;
}

void parseInputStr(char *inputStr, char **prgArgs) /* tokenizes input string and stores arguments in program args array */
{
  char *parsed = strtok(inputStr, " \n\t");
  int i = 0;

  while (parsed != NULL)
  {
    prgArgs[i] = parsed;
    parsed = strtok(NULL, " \n\t");
  }
}

void exe(char **prgArgs)
{
  int exitStatus = 0;

  pid_t pid;
  pid = fork();


  if (pid < 0)
  {
    fprintf(stderr, "Fork Failed\n");
    exit(-1);
  }

  else if (pid == 0)
  {
    execlp(*prgArgs, *prgArgs, NULL);
    fprintf(stderr, "Program Execution Failed\n");
    exit(-1);
  }

  else
  {
    while (1)
    {
      if (wait(&exitStatus) == pid)
      {
        return;
      }
    }
  }
}

int main(int argc, char **argv, char **envp)
{

const char* PATH = getenv("PATH");

printf("         _______                  _____                   _____                   _____                   _____          \n");
printf("        /::\\    \\                /\\    \\                 /\\    \\                 /\\    \\                 /\\    \\         \n");
printf("       /::::\\    \\              /::\\____\\               /::\\    \\               /::\\    \\               /::\\____\\        \n");
printf("      /::::::\\    \\            /:::/    /              /::::\\    \\             /::::\\    \\             /:::/    /        \n");
printf("     /::::::::\\    \\          /:::/    /              /::::::\\    \\           /::::::\\    \\           /:::/    /         \n");
printf("    /:::/~~\\:::\\    \\        /:::/    /              /:::/\\:::\\    \\         /:::/\\:::\\    \\         /:::/    /          \n");
printf("   /:::/    \\:::\\    \\      /:::/    /              /:::/__\\:::\\    \\       /:::/__\\:::\\    \\       /:::/____/           \n");
printf("  /:::/    / \\:::\\    \\    /:::/    /              /::::\\   \\:::\\    \\      \\:::\\   \\:::\\    \\     /::::\\    \\           \n");
printf(" /:::/____/   \\:::\\____\\  /:::/    /      _____   /::::::\\   \\:::\\    \\   ___\\:::\\   \\:::\\    \\   /::::::\\    \\   _____  \n");
printf("|:::|    |     |:::|    |/:::/____/      /\\    \\ /:::/\\:::\\   \\:::\\    \\ /\\   \\:::\\   \\:::\\    \\ /:::/\\:::\\    \\ /\\    \\ \n");
printf("|:::|____|     |:::|____|:::|    /      /::\\____/:::/  \\:::\\   \\:::\\____/::\\   \\:::\\   \\:::\\____/:::/  \\:::\\    /::\\____\\");
printf("\n");
printf(" \\:::\\   _\\___/:::/    /|:::|____\\     /:::/    \\::/    \\:::\\  /:::/    \\:::\\   \\:::\\   \\::/    \\::/    \\:::\\  /:::/    /\n");
printf("  \\:::\\ |::| /:::/    /  \\:::\\    \\   /:::/    / \\/____/ \\:::\\/:::/    / \\:::\\   \\:::\\   \\/____/ \\/____/ \\:::\\/:::/    / \n");
printf("   \\:::\\|::|/:::/    /    \\:::\\    \\ /:::/    /           \\::::::/    /   \\:::\\   \\:::\\    \\              \\::::::/    /  \n");
printf("    \\::::::::::/    /      \\:::\\    /:::/    /             \\::::/    /     \\:::\\   \\:::\\____\\              \\::::/    /   \n");
printf("     \\::::::::/    /        \\:::\\__/:::/    /              /:::/    /       \\:::\\  /:::/    /              /:::/    /    \n");
printf("      \\::::::/    /          \\::::::::/    /              /:::/    /         \\:::\\/:::/    /              /:::/    /     \n");
printf("       \\::::/____/            \\::::::/    /              /:::/    /           \\::::::/    /              /:::/    /      \n");
printf("        |::|    |              \\::::/    /              /:::/    /             \\::::/    /              /:::/    /       \n");
printf("        |::|____|               \\::/____/               \\::/    /               \\::/    /               \\::/    /        \n");
printf("         ~~                      ~~                      \\/____/                 \\/____/                 \\/____/         \n\n\n");


printf("Type the word exit or quit to exit Quash\n\n");

char inputLine[MAX_LENGTH];
char *inputArgs[32];

/* printf("PATH 0 = %s", envp[0]);
printf("\n");

printf("PATH 1 = %s", envp[1]);
printf("\n"); */

while (1)
{
  printf("Quash$ ");
  fgets(inputLine, MAX_LENGTH, stdin);

  printf("\n");
  parseInputStr(inputLine, inputArgs);

  if (exitQuash(inputArgs[0]))
  {
    printf("Exiting Quash...\n");
    exit(0);
  }

  else
  {
    exe(inputArgs);
  }
}

  return 0;
}
