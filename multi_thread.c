#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>

#define LOCK_INC(var) \
  do{\
    pthread_mutex_lock(&stat_lock);\
    var++;\
    pthread_mutex_unlock(&stat_lock);\
  }while(0)

#define NUM_CACL 100

pthread_mutex_t stat_lock = PTHREAD_MUTEX_INITIALIZER;
static int calc_success_count = 0;
static int calc_fail_count = 0;
static int accept_count = 0;

int nread(int fd, void *data, int size) {
  int ret, total = 0;
  while (total < size) {
    ret = read(fd, data + total, size - total);
    if (ret == -1) return -1;
    total += ret;
  }
}

int nwrite(int fd, void *data, int size) {
  int ret, total = 0;
  while (total < size) {
    ret = write(fd, data + total, size - total);
    if (ret == -1) return -1;
    total += ret;
  }
}

void *handle_client(void *arg) {
  int i, fd = *(int*)arg;
  if (nwrite(fd, &i, sizeof(int)) == -1) {
    perror("write");
    goto ERROR;
  }
  for (i = 0; i < NUM_CACL; i++) {
    int a, b, c;
    if (nread(fd, &a, sizeof(int)) == -1 || nread(fd, &b, sizeof(int)) == -1) {
      perror("read");
      LOCK_INC(calc_fail_count);
      goto ERROR;
    }
    a = ntohl(a);
    b = ntohl(b);
    c = htonl(a + b);
    if (nwrite(fd, &c, sizeof(int)) == -1) {
      perror("write");
      LOCK_INC(calc_fail_count);
      goto ERROR;
    }
    LOCK_INC(calc_success_count);
  }

ERROR:
  close(fd);
  free(arg);
  return NULL;
}

void handle_signal(int signum) {
  if (signum == SIGINT) {
    printf("\naccept connection %d\n"
        "calculate successfully %d\n"
        "calculate failed %d\n",
        accept_count,
        calc_success_count, calc_fail_count);
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
  if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1 || listen(listen_fd, 511) == -1) {
    perror("listen");
    exit(-1);
  }

  printf("listenning on port 1234...\n");

  while (1) {
    int client_fd, addrlen;
    struct sockaddr_in client_addr;
    addrlen = sizeof(client_addr);
    client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &addrlen);
    if (client_fd == -1) {
      perror("accept");
      continue;
    }
    accept_count++;
    int *fd = malloc(sizeof(int));
    *fd = client_fd;
    pthread_t thread;
    pthread_create(&thread, NULL, handle_client, (void*)fd);
  }
}
