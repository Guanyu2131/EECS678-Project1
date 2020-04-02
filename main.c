#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_LENGTH 1024
static int pipeFound = 0;
static int pipeIndex = 0;
static int ampersandFound=0;
static int ampersandIndex=0;
static int numArgs = 0; // modified in parseInputStr
static int hasRedirect = 0;
static int redirectIndex = 0;

struct Process
{
  int id;
  int pid;
  char* command;
};
static int totalJobs;
static int nextId;
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
  printf("Running Processes:\n");
  for (int i = 0; i < totalJobs; i++) {
    pid_t pid=getpid();
    printf("[%d] PID: %d, COMMAND: %s\n", myJobs[i].id,  myJobs[i].pid,  myJobs[i].command);
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

    if (strcmp(prgArgs[i], "&") == 0)
    {
      ampersandFound = 1;
      ampersandIndex = i;
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
  // printf("%s\n", prgArgs[0]);
  // //printf("%s\n", prgArgs[1]);
  // //printf("%s\n", prgArgs[2]);
  // printf("\n");

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
    if (prgArgs[1] == NULL)
    {
      execlp(*prgArgs, *prgArgs, NULL);
      fprintf(stderr, "Program Execution (without args) Failed\n");
      exit(0);
    }
    else
    {
      execvp(prgArgs[0], prgArgs);
      fprintf(stderr, "Program Execution (with args) Failed\n");
      //printf("%s\n", prgArgs[0]);
      exit(0);
    }
  }

  else // parent
  {
    waitpid(pid, &exitStatus, 0);
  }
}

void runBackground(char **inputArgs)
{
  if (ampersandIndex != 0)
  {
    char* cmd[ampersandIndex];
    cmd[ampersandIndex]=NULL;
    for (int i = 0; i < ampersandIndex; i++)
    {
      cmd[i] = inputArgs[i];
      while((cmd[i][strlen(cmd[i])-1]==' ' || cmd[i][strlen(cmd[i])-1]=='\t')){
        cmd[i][strlen(cmd[i])-1] = '\0';
      }
    }

    int exitStatus;
    pid_t pid;
    totalJobs++;
    myJobs[totalJobs-1].command="";
    pid=fork();
    if (pid < 0){ //error
      fprintf(stderr, "Fork Failed for run process in background\n");
      exit(-1);
    }
    else if(pid==0){ //child
      printf("[%d] PID: %d running in background\n", nextId, getpid());
      sleep(2.5);
      exe(cmd);
      printf("\n[%d] PID: %d finished COMMAND: %s\n\nQuash$ ", nextId, getpid(), cmd[0]);
      exit(0);

    }
    else{ //parent
      myJobs[totalJobs-1].id=nextId;
      myJobs[totalJobs-1].pid=pid;
      //
      nextId++;
      waitpid(pid, &exitStatus, SIGCHLD);

    }
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
      while((leftCmd[i][strlen(leftCmd[i])-1]==' ' || leftCmd[i][strlen(leftCmd[i])-1]=='\t')){
        leftCmd[i][strlen(leftCmd[i])-1] = '\0';
      }
    }

    int j = 0;
    for (int i = pipeIndex + 1; i < numArgs; i++)
    {
      rightCmd[j] = args[i];
      while((rightCmd[j][strlen(rightCmd[j])-1]==' ' || rightCmd[j][strlen(rightCmd[j])-1]=='\t')){
        leftCmd[i][strlen(leftCmd[i])-1] = '\0';
      }
      j++;
    }


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
      dup2(fds[1], STDOUT_FILENO);
      close(fds[0]);
      exe(leftCmd);
      exit(0);
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
      dup2(fds[0], STDIN_FILENO);
      close(fds[1]);
      exe(rightCmd);
      exit(0);
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
  totalJobs=1;
  nextId=1;
  myJobs[0].id=nextId;
  myJobs[0].pid=getpid();
  myJobs[0].command="quash";
  nextId++;

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

        else if (ampersandFound)
        {
          runBackground(inputArgs);
          ampersandFound = 0;
          ampersandIndex = 0;
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
