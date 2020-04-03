#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_LENGTH 1024
static int pipeFound = 0;
static int pipeIndex = 0;
static int ampersandFound=0;
static int ampersandIndex=0;
static int numArgs = 0; // modified in parseInputStr
static int hasRedirect = 0;
static int redirectIndex = 0;
static char redirectSymbol = '\0';
static pid_t processID;
char *inputLineCopy;
//int EXIT_QUASH = 0;

struct Process
{
  int id;
  int pid;
  char* commandString;
};
static int totalJobs;
static int nextId;
static struct Process myJobs[MAX_LENGTH];

int runCmdFromFile(char *cmdFromFile);


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
    printf("[%d] PID: %d, COMMAND: %s\n", myJobs[i].id,  myJobs[i].pid,  myJobs[i].commandString);
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
      redirectIndex = i;

      if (strcmp(prgArgs[i], "<") == 0)
        redirectSymbol = '<';
      else
        redirectSymbol = '>';
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

void exePid(char **prgArgs, pid_t pid)
{
  int exitStatus;

  if (pid < 0) // error message
  {
    fprintf(stderr, "Fork Failed\n");
    exit(-1);
  }

  else if (pid == 0) // child
  {
    if (prgArgs[1] == NULL)
    {
      execvp(prgArgs[0], prgArgs);
      fprintf(stderr, "Program Execution (without args) Failed\n");
      exit(0);
    }
    else
    {
      execvp(prgArgs[0], prgArgs);
      fprintf(stderr, "Program Execution (with args) Failed\n");
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
  if (ampersandIndex > 0 && strcmp(inputArgs[0], "jobs")!=0)
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
    ampersandFound=0;


    int exitStatus;
    pid_t pid;


    pid=fork();
    processID=pid;

    if (pid < 0){ //error
      fprintf(stderr, "Fork Failed for run process in background\n");
      exit(-1);
    }
    else if(pid==0){ //child
      //sid=setsid();
      printf("[%d] PID: %d running in background\n", nextId, getpid());
      sleep(2.5);
      exe(cmd);
      printf("\n[%d] PID: %d finished COMMAND: %s\n\nQuash$ ", nextId, getpid(), cmd[0]);
      exit(exitStatus);

    }
    else{ //parent
      totalJobs++;
      myJobs[totalJobs-1].id=nextId;
      myJobs[totalJobs-1].pid=pid;
      nextId++;
      //waitpid(pid, &exitStatus, 0);
    }

  }
}

void makePipe()
{
  if (pipeIndex != 0 && pipeIndex != numArgs)
  {
    char *leftCmd[50];
    char *rightCmd[50];

    char *parseLeft = strtok(inputLineCopy, "|");
    char *parseRight = strtok(NULL, "\0");
    int i = 0;
    int j = 0;

    char* parser = strtok(parseLeft, " \n\t");

    while (parser != NULL)
    {
      leftCmd[i] = parser;
      parser = strtok(NULL, " \n\t");
      i++;
    }

    if (parser == NULL)
    {
      leftCmd[i] = NULL;
    }

    parser = strtok(parseRight, " \n\t");

    while (parser != NULL)
    {
      rightCmd[j] = parser;
      parser = strtok(NULL, " \n\t");
      j++;
    }

    if (parser == NULL)
    {
      rightCmd[j] = NULL;
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
      exePid(leftCmd, pid1);
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
      exePid(rightCmd, pid2);
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

void redirect()
{
  if (redirectIndex != 0 && redirectIndex != numArgs)
  {
    char *leftCmd[50];
    char *rightCmd[50];

    char *parseLeft = strtok(inputLineCopy, "<>");
    char *parseRight = strtok(NULL, "\0");
    int i = 0;
    int j = 0;

    char* parser = strtok(parseLeft, " \n\t");

    while (parser != NULL)
    {
      leftCmd[i] = parser;
      parser = strtok(NULL, " \n\t");
      i++;
    }

    if (parser == NULL)
    {
      leftCmd[i] = NULL;
    }

    parser = strtok(parseRight, " \n\t");

    while (parser != NULL)
    {
      rightCmd[j] = parser;
      parser = strtok(NULL, " \n\t");
      j++;
    }

    if (parser == NULL)
    {
      rightCmd[j] = NULL;
    }

    if (redirectSymbol == '>')
    {
      int status;

      pid_t pid;
      pid = fork();

      if (pid < 0)
      {
        fprintf(stderr, "Fork Failed!\n");
        exit(-1);
      }

      else if (pid == 0)
      {
        char *rightCmdName = rightCmd[0];
        //printf("%s\n", rightCmdName);
        int outfd = open(rightCmdName, O_CREAT | O_WRONLY | O_TRUNC, 0666);

        dup2(outfd, STDOUT_FILENO);
        exe(rightCmd);

        close(outfd);
        exit(0);
      }

      else
      {
        waitpid(pid, &status, 0);
      }
    }

    else if (redirectSymbol == '<')
    {
      if (strcmp(leftCmd[0], "quash") == 0)
      {
        //pid_t myPid;
        //myPid = fork();
        //  int stat;
        char *fileName = rightCmd[0];
          FILE *filePtr;
          char *cmdFromFile;
          filePtr = fopen(fileName, "r");
          //int returnStatus = 0;

          if (filePtr == NULL)
          {
            fprintf(stderr, "Command file not found!\n");
            fclose(filePtr);
            //exit(-1);
          }

          while (fgets(cmdFromFile, MAX_LENGTH, filePtr))
          {
            runCmdFromFile(cmdFromFile);
          }
          fclose(filePtr);
      }

      else
      {
        pid_t myPid;
        myPid = fork();
        int stat;

        if (myPid < 0)
        {
          fprintf(stderr, "Fork failed in '<' if-else block\n");
          exit(-1);
        }

        else if (myPid == 0)
        {
          char *fileName = rightCmd[0];
          int infd;
          infd = open(fileName, O_RDONLY);
          dup2(infd, STDIN_FILENO);
          exePid(leftCmd, myPid);
          close(infd);
          exit(0);
        }

        else
        {
          waitpid(myPid, &stat, 0);
        }
      }
    }
  }

  else
  {
    printf("Unable to redirect!\n");
  }
}

int runCmdFromFile(char *cmdFromFile)
{
  cmdFromFile[strlen(cmdFromFile)-1] = '\0';

  while((cmdFromFile[strlen(cmdFromFile)-1]==' ' || cmdFromFile[strlen(cmdFromFile)-1]=='\t'))
    cmdFromFile[strlen(cmdFromFile)-1] = '\0';

  inputLineCopy = strdup(cmdFromFile);

  char *cmdArgs[100];
  parseInputStr(cmdFromFile, cmdArgs);

  int exitStatus;
  pid_t returnPid=waitpid(processID, &exitStatus, WNOHANG);
  if(returnPid==processID){
    totalJobs--;
    while(totalJobs>1){
      totalJobs--;
    }
  }

  if (exitQuash(cmdArgs[0]))
  {
    printf("Exiting Quash...\n");
    exit(0);
  }

  else if (checkJobs(cmdArgs[0])){
    jobs(cmdArgs[1]);
  }

  else if (checkChangeDirectory(cmdArgs[0])){
    changeDirectory(cmdArgs[1]);
  }

  else if (strcmp(cmdArgs[0], "set") == 0)
  {
    setPath(cmdArgs[1]);
  }

  else if (pipeFound)
  {
    makePipe();
    pipeFound = 0;
    pipeIndex = 0;
  }

  else if (ampersandFound)
  {
    char *bgProcess=strdup(cmdArgs[0]);
    myJobs[totalJobs].commandString=bgProcess;
    runBackground(cmdArgs);
    ampersandFound = 0;
    ampersandIndex = 0;
  }

  else if (hasRedirect)
  {
    redirect();
    hasRedirect = 0;
    redirectIndex = 0;
    redirectSymbol = '\0';
  }

  else
  {
    exe(cmdArgs);
  }

  return 0;
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
  myJobs[0].commandString="./quash";
  nextId++;

  while (1)
  {
      /*if (EXIT_QUASH == 1)
      {
        printf("Exiting Quash...\n");
        exit(0);
      }*/

      printf("Quash$ ");
      fgets(inputLine, MAX_LENGTH, stdin);

      inputLine[strlen(inputLine)-1] = '\0';

      while((inputLine[strlen(inputLine)-1]==' ' || inputLine[strlen(inputLine)-1]=='\t')){
        inputLine[strlen(inputLine)-1] = '\0';
      }

      inputLineCopy = strdup(inputLine);

      if(strlen(inputLine)!=0){
        parseInputStr(inputLine, inputArgs);

        printf("\n");
        int exitStatus;
        pid_t returnPid=waitpid(processID, &exitStatus, WNOHANG);
        if(returnPid==processID){
          totalJobs--;
          while(totalJobs>1){
            totalJobs--;
          }
        }
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
          makePipe();
          pipeFound = 0;
          pipeIndex = 0;
        }

        else if (ampersandFound)
        {
          char *bgProcess=strdup(inputArgs[0]);
          myJobs[totalJobs].commandString=bgProcess;
          runBackground(inputArgs);
          ampersandFound = 0;
          ampersandIndex = 0;
        }

        else if (hasRedirect)
        {
          redirect();
          hasRedirect = 0;
          redirectIndex = 0;
          redirectSymbol = '\0';
        }

        else
        {
          exe(inputArgs);
        }
      }
    }

  return 0;
}
