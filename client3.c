#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define PORT 9000

int main(int argc, char const* argv[]) {

 int sock = 0;
 int valread;

 struct sockaddr_in serv_addr;

 sock = socket(AF_INET, SOCK_STREAM, 0);

 serv_addr.sin_family = AF_INET;
 serv_addr.sin_port = htons(PORT);
 inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

 connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

 while(1) {
  printf("\nՓ\t");
  char comd[100];
  fgets(comd, sizeof(char)*1024, stdin);
  comd[strlen(comd)-1] = '\0';

  send(sock, comd, sizeof(comd), 0);

  if(strcmp(comd, "crd") == 0 || strcmp(comd, "rmd") == 0 || strcmp(comd, "cct") == 0) {
   char n[100];
   fgets(n, sizeof(n), stdin);
   n[strlen(n)-1] = '\0';
   send(sock, n, strlen(n), 0);
  }

  char buffer[1024] = {0};
  valread = recv(sock, buffer, sizeof(buffer), 0);

  printf("%s", buffer);
 }

 close(sock);

 return 0;
}
