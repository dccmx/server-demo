#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

void do_a_plus_b(int a, int b) {
  int fd, c, t, on = 1;
  struct sockaddr_in server_addr;
  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
    perror("listen");
    exit(-1);
  }
  memset((void *) &server_addr, 0, sizeof(struct sockaddr_in));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_addr.sin_port = htons(1234);
  if (connect(fd, (const struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    perror("client");
    return;
  }
  t = htons(a);
  write(fd, &t, sizeof(int));
  t = htons(b);
  write(fd, &t, sizeof(int));
  read(fd, &c, sizeof(int));
  c = ntohs(c);
  if (c != a + b) {
    fprintf(stderr, "error: %d+%d=%d\n", a, b, c);
  }

  close(fd);
}

int main(int argc, char *argv[]) {
  do_a_plus_b(100, 200);
}
