#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void print_argsarray(char ** argsarray){
  printf("argsarray > ");
  int i = 0;
  while (argsarray[i]){
    printf("%s ", argsarray[i]);
    i++;
  }
  printf("\n");
}
void execute(char ** argsarray, int * pipefd, int direction, int * pid){
  //print out our array
  *pid = fork();
  if (*pid == 0) {
    if (pipefd != 0 && direction == 1){
      close(pipefd[0]);
      dup2(pipefd[1], STDOUT_FILENO);
      close(pipefd[1]);
    } 
    if (pipefd != 0 && direction == 0){
      close(pipefd[1]);
      dup2(pipefd[0], STDIN_FILENO);
      close(pipefd[0]);
    }
    execv(argsarray[0], argsarray);
    exit(EXIT_SUCCESS);
  } else {
    // int status;
    // wait(&status);
    if (pipefd){
      close(pipefd[direction]);
    }
    // printf("pid:%d status:%d\n", pid, WEXITSTATUS(status));
    // return WEXITSTATUS(status);
  }
}
void test_piped2(){
  int pipefd[2];
  char *before[] = {"/bin/ls", "-l", (char*)0};
  // char *after[] = {"/bin/cat", (char*)0};
  char *after[] = {"/usr/bin/grep", "hw", (char*)0};
  if (pipe(pipefd) == -1){
    perror("Pipe Failed");
    exit(EXIT_FAILURE);
  }
  int pid1, pid2, status1, status2;
  execute(before, pipefd, 1, &pid1);
  execute(after, pipefd, 0, &pid2);
  waitpid(pid1, &status1, 0);
  waitpid(pid2, &status2, 0);
  printf("pid:%d status:%d\n", pid1, WEXITSTATUS(status1));
  printf("pid:%d status:%d\n", pid2, WEXITSTATUS(status2));

}
char ** init_argsarray(char ** argsarray, int n, int m){
  if (argsarray != NULL) {
    for(int i = 0; i < n; i++){
      free(argsarray[i]);
    }
    free(argsarray);
  }
  argsarray = (char**)malloc(n * sizeof(char*));
  for(int i = 0; i < n; i++){
    argsarray[i] = (char*)malloc(m * sizeof(char));
  }
  return argsarray;
}
int prompt(){
  	char line[500];
 	char ** argsarray = NULL;
	argsarray = init_argsarray(argsarray, 20, 100);
  
	printf("CS361 > ");
  	//read lined from terminal
 	fgets(line, 500, stdin);
  
  //break the string up into words
  char *word = strtok(line, " ");
  int i = 0;
  int * pipefd = 0;
  int pid, pid2;
  int status, status2;
  while (word) {
    if (word[strlen(word) - 1] == '\n') {
    	word[strlen(word) - 1] = '\0';
    }
    if (strcmp(word, "exit") == 0 && i == 0) {
	    exit(0);
    } else if (strcmp(word, ";") == 0){
      //null terminate the args array and execute the first command
      argsarray[i] = (char *) 0;
      execute(argsarray, 0, 0, &pid);
      waitpid(pid, &status, 0);
      printf("pid:%d status:%d\n", pid, WEXITSTATUS(status));
      //reinit the argsarray and start again
      argsarray = init_argsarray(argsarray, 20, 100);
      i = 0;
    } else if (strcmp(word,"|") == 0){
      argsarray[i] = (char *) 0;
      pipefd = (int*)malloc(2 * sizeof(int));
      if (pipe(pipefd) == -1){
        perror("Pipe Failed");
        exit(EXIT_FAILURE);
      }
      execute(argsarray, pipefd, 1, &pid2);
      argsarray = init_argsarray(argsarray, 20, 100);
      i = 0;
    } else {
      //copy a word to the arg array
      strcpy(argsarray[i], word);
      //get next word
      i = i + 1;
    }
    word = strtok(NULL, " ");
  }
  argsarray[i] = (char *) 0;
  execute(argsarray, pipefd, 0, &pid);
  if (pipefd) {
    waitpid(pid2, &status2, 0);
    printf("pid:%d status:%d\n", pid2, WEXITSTATUS(status2));
  }
  waitpid(pid, &status, 0);
  printf("pid:%d status:%d\n", pid, WEXITSTATUS(status));
  return 0;
}
void INTHandler(int sig){
  signal(sig, SIG_IGN);
  printf("caught sigint\n");
  return;
}
void TSTPHandler(int sig){
  signal(sig, SIG_IGN);
  printf("caught sigtstp\n");
  return;
}
int main(){
  signal(SIGINT, INTHandler);
  signal(SIGTSTP, TSTPHandler);
  while (1) {
	  prompt();
  }
  // test_piped2();
	return 0;
}

