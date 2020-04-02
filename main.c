#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_LENGTH 1024
static int pipeFound = 0;
static int pipeIndex = 0;
static int numArgs = 0; // modified in parseInputStr
static int hasRedirect = 0;
static int redirectIndex = 0;

struct Process
{
  int id;
  int pid;
  char* command;
};
static int totalJobs=1;
static struct Process myJobs[MAX_LENGTH];

int exitQuash(char *cmd)
{
  if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0)
  {
    return 1;
  }

  return 0;
}


int checkJobs(char *cmd)
{
  if (strcmp(cmd, "jobs") == 0)
  {
    return 1;
  }
  else{
    return 0;
  }
}

int checkChangeDirectory(char *cmd)
{
  if (strcmp(cmd, "cd") == 0)
  {
    return 1;
  }
  else{
    return 0;
  }
}

void changeDirectory(char *directory){
  //printf("%s\n", directory);
  if(directory==NULL){
    printf("Going Home\n");
    chdir(getenv("HOME"));
  }
  else{
    if(chdir(directory)==0){
      printf("Changed current directory to %s\n", directory);
    }
    else if(strcmp(directory, "~") == 0 || strcmp(directory, "HOME") == 0){
      printf("Going Home\n");
      chdir(getenv("HOME"));
    }
    else{
      printf("Directory does not exist!\n");
    }
  }
}

void jobs(char *directory){
  for (int i = 0; i < totalJobs; i++) {
    pid_t pid=getpid();
    printf("[%d] PID: %d, COMMAND: %s\n", myJobs[i].id,  myJobs[i].pid,  myJobs[i].command);
    //pid_t ppid=getppid();
    //printf("PPID:%d\n", ppid);
  }

}

void setPath(char *path)
{
  char *var = strtok(path, "=");
  char *pathToSet = strtok(NULL, "\0");

  int success = setenv(var, pathToSet, 1);

  if (success < 0)
  {
    printf("Unable to set path for %s!\n", var);
  }
}

void parseInputStr(char *inputStr, char **prgArgs) /* tokenizes input string and stores arguments in program args array */
{
  char *parsed = strtok(inputStr, " \n\t");
  int i = 0;
  numArgs = 0;

  while (parsed != NULL)
  {
    prgArgs[i] = parsed;

    if (strcmp(prgArgs[i], "<") == 0 || strcmp(prgArgs[i], ">") == 0)
    {
      hasRedirect = 1;
    }

    if (strcmp(prgArgs[i], "|") == 0)
    {
      pipeFound = 1;
      pipeIndex = i;
    }

    parsed = strtok(NULL, " \n\t");
    i++;
  }

  if (parsed == NULL)
  {
    prgArgs[i] = NULL;
  }

  numArgs = i - 1;
}

void exe(char **prgArgs)
{
  // For testing
  /*printf("%s\n", prgArgs[0]);
  printf("%s\n", prgArgs[1]);
  printf("%s\n", prgArgs[2]);
  printf("\n");*/

  int exitStatus;

  pid_t pid;
  pid = fork();

  if (pid < 0) // error message
  {
    fprintf(stderr, "Fork Failed\n");
    exit(-1);
  }

  else if (pid == 0) // child
  {
    totalJobs++;
    myJobs[totalJobs-1].id=totalJobs-1;
    myJobs[totalJobs-1].pid=getpid();
    myJobs[totalJobs-1].command="";
    if (prgArgs[1] == NULL)
    {
      execlp(*prgArgs, *prgArgs, NULL);
      fprintf(stderr, "Program Execution (without args) Failed\n");
      exit(-1);
    }
    else // parent
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

void makePipe(char **args)
{
  if (pipeIndex != 0 && pipeIndex != numArgs)
  {
    char *leftCmd[50];
    char *rightCmd[50];

    for (int i = 0; i < pipeIndex; i++)
    {
      leftCmd[i] = args[i];
      //printf("%s\n", leftCmd[i]);
      //strcat(leftCmd[i], '\0', 1);
      //printf("Left arg %d: %s\n", i, leftCmd[i]);
    }

    //printf("\n\n\n");

    leftCmd[pipeIndex] = "\0";
    leftCmd[49] = "\0";

    int j = 0;

    for (int i = pipeIndex + 1; i < numArgs + 1; i++)
    {
      rightCmd[j] = args[i];
      //printf("%s\n", rightCmd[j]);
      //strcat(rightCmd[i], '\0', 1);
      j++;
    }

    rightCmd[numArgs + 1] = "\0";
    rightCmd[49] = "\0";

    int fds[2];
    pipe(fds);

    pid_t pid1;
    pid_t pid2;
    pid1 = fork();

    int exitStatus1;
    int exitStatus2;

    if (pid1 < 0)
    {
      fprintf(stderr, "Fork Failed for makePipe pid1\n");
      exit(-1);
    }

    else if (pid1 == 0)
    {
      close(fds[0]);
      dup2(fds[1], STDOUT_FILENO);
      close(fds[1]);
      exe(leftCmd);
      exit(-1);

      /*if (execvp(leftCmd[0], leftCmd) < 0)
      {
        fprintf(stderr, "Error executing left command!\n");
        exit(-1);
      }*/
    }

    else
    {
      waitpid(pid1, &exitStatus1, 0);
    }

    pid2 = fork();

    if (pid2 < 0)
    {
      fprintf(stderr, "Fork Failed for makePipe pid2\n");
      exit(-1);
    }

    else if (pid2 == 0)
    {
      close(fds[1]);
      dup2(fds[0], STDIN_FILENO);
      close(fds[0]);
      exe(rightCmd);
      exit(-1);

      /*if (execvp(rightCmd[0], rightCmd) < 0)
      {
        fprintf(stderr, "Error executing right command!\n");
        exit(-1);
      }*/
    }

    else
    {
      waitpid(pid2, &exitStatus2, 0);
      close(fds[0]);
      close(fds[1]);
    }
  }

  else
  {
    printf("Syntax Error: Invalid use of '|' command.\n");
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
  myJobs[0].id=0;
  myJobs[0].pid=getpid();
  myJobs[0].command="quash";


  while (1)
  {
      printf("Quash$ ");
      fgets(inputLine, MAX_LENGTH, stdin);
      inputLine[strlen(inputLine)-1] = '\0';

      while((inputLine[strlen(inputLine)-1]==' ' || inputLine[strlen(inputLine)-1]=='\t')){
        inputLine[strlen(inputLine)-1] = '\0';
      }

      if(strlen(inputLine)!=0){
        parseInputStr(inputLine, inputArgs);

        printf("\n");

        if (exitQuash(inputArgs[0]))
        {
          printf("Exiting Quash...\n");
          exit(0);
        }

        else if (checkJobs(inputArgs[0])){
          jobs(inputArgs[1]);
        }


        else if (checkChangeDirectory(inputArgs[0])){
          changeDirectory(inputArgs[1]);
        }

        else if (strcmp(inputArgs[0], "set") == 0)
        {
          setPath(inputArgs[1]);
        }

        else if (pipeFound)
        {
          makePipe(inputArgs);
          pipeFound = 0;
          pipeIndex = 0;
        }

        else if (hasRedirect)
        {
          hasRedirect = 0;
        }

        else
        {
          exe(inputArgs);
        }
      }
    }

  return 0;
}
