#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <pthread.h>

#define PORT 9000
#define DATA_SIZE 1024
#define NUM 50

void *connection_handler(void *);
void *client(void* new_socket);
void ls();
void lsl();
void pwd();
void help();
void whoami();
int prl();
void *threading(void *);

int main(int argc, char const* argv[]) {

 int count = 0;
 char *folder;
 int server_sock;
 int new_sock[NUM];
 struct sockaddr_in address;
 int addrlen = sizeof(address);
 server_sock = socket(AF_INET, SOCK_STREAM, 0);

 address.sin_family = AF_INET;
 address.sin_addr.s_addr = htonl(INADDR_ANY);
 address.sin_port = htons(PORT);

 time_t tick;
 pthread_t thread[NUM];
 pthread_attr_t attr;
 pthread_attr_init(&attr);
 pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

 bind(server_sock, (struct sockaddr*)&address, sizeof(address));
 struct sockaddr_in newaddr;

 listen(server_sock, 1);

 while(1) {

  new_sock[count] = accept(server_sock,(struct sockaddr*)&address, (socklen_t*)&addrlen);

  if(new_sock[count] < 0)
   continue;
   if(pthread_create(&thread[count], &attr, threading, &new_sock[count]) < 0) {
    perror("Error while creating thread");
    return 1;
   }
   count++;
   tick = time(NULL);

 }
 close(server_sock);
 return 0;
}

void *threading(void *args) {

 FILE *pointer;
 int valread;
 int new_sock = *(int*) args;

  while(1) {
   char buffer[1024] = {0};
   valread = recv(new_sock, buffer, sizeof(buffer), 0);
   printf("received %s\n", buffer);

   pid_t pid;
   pid = fork();

   if(pid < 0) {
    perror("1");
   }
   if(pid == 0) {
    int file = open("output", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    close(1);
    dup(file);

    if(strcmp (buffer, "df") == 0) { // Command 1
     pid_t pid;
     pid = fork();

     if(pid == 0) {
      ls();
     } else if(pid > 0) {
      wait(NULL);
     }

    } else if(strcmp(buffer, "crt") == 0) { // Command 2
     pid_t pid;
     pid = fork();

     if(pid == 0) {
     pwd();
     } else if(pid > 0) {
     wait(NULL);
     }

    } else if(strcmp(buffer, "sos") == 0) { // Command 3
     help();
    } else if(strcmp(buffer, "crd") == 0) { // Command 4
     char name[100];
     int valread2 = read(new_sock, name, 100);

     char* arr[] = {"mkdir", name, NULL};
     execvp("mkdir", arr);

    } else if(strcmp(buffer , "rmd") == 0) { // Command 5
      char name[100];
      int valread2 = read(new_sock, name, 100);

      char* arr[] = {"rmdir", name, NULL};
      execvp("rmdir", arr);

    } else if(strcmp(buffer, "cct") == 0) { // Command 6
      char name[100], c;
      int valread2 = read(new_sock, name, 100);

      pointer = fopen(name, "r");

      if(pointer == NULL) {
      char emp[15] = "File is empty\n";
      exit(0);
      }

      c = fgetc(pointer);

      while(c != EOF){ //End of File
       printf("%c", c);
       c = fgetc(pointer);
      }
      fclose(pointer);

    } else if(strcmp(buffer, "whoisyou") == 0) { // Command 7
     whoami();
    } else if(strcmp(buffer, "lsl") == 0) { // Command 8
     pid_t pid;
     pid = fork();

     if(pid == 0) {
      lsl();
     } else if(pid > 0) {
      wait(NULL);
     }

    } else if(strcmp(buffer, "prl") == 0) { // Command 9
     prl();
    } else {
     printf("Command does not exist\n");
   }

    exit(0);
   }
   if(pid > 0) {
    wait(NULL);
    FILE *pointer = fopen("output", "r");
    char bf1[1024] = {0};
    char bf2[1024] = {0};

    while(fgets(bf2, 1024, pointer) != NULL) {
     strcat(bf1, bf2);
    }

    send(new_sock, bf1, sizeof(bf1), 0);
    printf("output sent to client\n");
   }

  }
  close (new_sock);

 return NULL;
}

void ls() {
 char *exp[] = {"ls", ".", NULL};
 execvp("ls", exp);
}

void pwd() {
 char *exp[] = {"pwd", "-P", NULL};
 execvp("pwd", exp);
}

void help() {
 printf("Welcome to our Shell, the available commands we have are:\n");
 printf("~df\t Displays the files in your current directory,\n~crt\t Prints your current directory,\n");
 printf("~crd\t Creates a new directory,\n~rmd\t Removes a directory\n");
 printf("~cct\t Displays the content of a file,\n~whoisyou\t Displays the name of the user\n");
 printf("~lsl\t Displays the files of the current directory in long format,\n~prl\t Prints number of current root processes");
} //don't forget to add the other commands

void lsl() {
 execlp("ls", "ls", "-l", NULL);
}

void whoami() {
 char *p=getenv("USER");

  if(p == NULL) {
   return;
  }
  printf("%s\n", p);
}

int prl() {
 int parent_child_fd[2];
 pid_t child, grandchild;

 if (pipe(parent_child_fd) == -1) {
  perror("Error creating parent_child_fd");
  return EXIT_FAILURE;
 }

 if ((child = fork()) == -1) {
  perror("Error forking child");
  return EXIT_FAILURE;
 } else if(child == 0){
    int child_grandchild_fd[2];

  if (pipe(child_grandchild_fd) == -1) {
   perror("Error creating child_grandchild_fd");
   exit(EXIT_FAILURE);
  }

  close(parent_child_fd[1]);

  dup2(parent_child_fd[0],0);
  close(parent_child_fd[0]);

  if ((grandchild = fork()) == -1) {
   perror("Error forking child");
   exit(EXIT_FAILURE);
  } else if(grandchild == 0){
   close(child_grandchild_fd[1]);
   dup2(child_grandchild_fd[0],0);

   execlp("/usr/bin/wc", "/usr/bin/wc", "-l", (char*)NULL);

   perror("Failed in wc");
   exit(EXIT_FAILURE);
  }

  dup2(child_grandchild_fd[1],1);
  close(child_grandchild_fd[0]);
  close(child_grandchild_fd[1]);

  execlp("grep", "grep", "root", (char *)NULL);
  perror("Failed in grep");
  exit(EXIT_FAILURE);
 }

 close(parent_child_fd[0]);
 dup2(parent_child_fd[1],1);
 close(parent_child_fd[1]);

 sleep(1);
 execlp("ps", "ps", "-ef", (char *)NULL);
 perror("Failed in ps");

 return -1;
}
