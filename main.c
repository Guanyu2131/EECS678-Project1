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


int checkChangeDirectory(char *cmd)
{
  if (strcmp(cmd, "cd") == 0)
  {
    return 1;
  }

  return 0;
}

void changeDirectory(char *directory){
  //printf("%s\n", directory);
  if(directory=="~"||directory==""||directory=="home"){
    printf("Going Home\n");
    chdir(getenv("HOME"));
  }
  else{
    if(chdir(directory)==0){
      printf("Changed current directory to %s\n", directory);
    }
    else{
      printf("Directory does not exist!\n");
    }
  }
}

void setPath(char *path) // Work on this
{
  return;
}

void parseInputStr(char *inputStr, char **prgArgs) /* tokenizes input string and stores arguments in program args array */
{
  char *parsed = strtok(inputStr, " \n\t");
  int i = 0;

  while (parsed != NULL)
  {
    prgArgs[i] = parsed;
    parsed = strtok(NULL, " \n\t");
    i++;
  }

  if (parsed == NULL)
  {
    prgArgs[i] = NULL;
  }
}

void exe(char **prgArgs)
{
  int exitStatus = 0;

  pid_t pid;
  pid = fork();


  if (pid < 0) // error message
  {
    fprintf(stderr, "Fork Failed\n");
    exit(-1);
  }

  else if (pid == 0) // child
  {
    if (prgArgs[1] == NULL)
    {
      execlp(*prgArgs, *prgArgs, NULL);
      fprintf(stderr, "Program Execution (without args) Failed\n");
      exit(-1);
    }

    else
    {
      execvp(prgArgs[0], prgArgs);
      fprintf(stderr, "Program Execution (with args) Failed\n");
      exit(-1);
    }
  }

  else // parent
  {
    waitpid(pid, &exitStatus, 0);
  }
}

int main(int argc, char **argv, char **envp)
{
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
  printf("|:::|____|     |:::|____|:::|    /      /::\\____/:::/  \\:::\\   \\:::\\____/::\\   \\:::\\   \\:::\\____/:::/  \\:::\\    /::\\____\\\n");
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

  char inputLine[MAX_LENGTH]; // command line
  char *inputArgs[100]; // args for command

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

    else if (checkChangeDirectory(inputArgs[0])){
      changeDirectory(inputArgs[1]);
    }

    else if (strcmp(inputArgs[0], "set") == 0)
    {
      setPath(inputArgs[1]);
    }

    else
    {
      exe(inputArgs);
    }
  }

  return 0;
}
