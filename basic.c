#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>

static int calc_count = 0;

void handle_client(int fd) {
  int a, b, c;
  read(fd, &a, sizeof(int));
  read(fd, &b, sizeof(int));
  a = ntohs(a);
  b = ntohs(b);
  c = htons(a + b);
  write(fd, &c, sizeof(int));
  close(fd);
  calc_count++;
}

void handle_signal(int signum) {
  if (signum == SIGINT) {
    printf("totally %d calculations\nshutdown now...\n", calc_count);
    exit(0);
  }
}

int main() {
  int listen_fd, on = 1;
  struct sockaddr_in server_addr;

  signal(SIGINT, handle_signal);

  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
    perror("listen");
    exit(-1);
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(1234);
  if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1 || listen(listen_fd, 5) == -1) {
    perror("listen");
    exit(-1);
  }

  printf("listenning on port 1234...\n");

  while (1) {
    int client_fd, addrlen;
    struct sockaddr_in client_addr;
    addrlen = sizeof(client_addr);
    client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &addrlen);
    handle_client(client_fd);
  }
}
