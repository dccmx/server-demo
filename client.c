#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#include "common.h"

#define LOCK_INC(var) \
  do{\
    pthread_mutex_lock(&stat_lock);\
    var++;\
    pthread_mutex_unlock(&stat_lock);\
  }while(0)


pthread_mutex_t init_count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int init_count = 0;
int total_client = 1000;
static pthread_t threads[1000];
const char *server_ip;

pthread_mutex_t stat_lock = PTHREAD_MUTEX_INITIALIZER;
static int calc_success_count = 0;
static int calc_count = 0;
static int conn_success_count = 0;
static int conn_fail_count = 0;

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

void *do_a_plus_b(void *arg) {
  int fd, i, on = 1;
  struct sockaddr_in server_addr;

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1 || setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
    perror("create socket");
    goto ERROR;
  }
  memset((void *) &server_addr, 0, sizeof(struct sockaddr_in));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(server_ip);
  server_addr.sin_port = htons(PORT);

  pthread_mutex_lock(&init_count_mutex);
  init_count++;
  if (init_count == total_client) {
    fprintf(stderr, "\nGO!!!\n");
    pthread_cond_broadcast(&cond);
  } else {
    fprintf(stderr, "wait[%-3d %3d] ", init_count, fd);
    if (init_count % 10 == 0) fprintf(stderr, "\n");
    pthread_cond_wait(&cond, &init_count_mutex);
  }
  pthread_mutex_unlock(&init_count_mutex);

  if (connect(fd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
    perror("connect");
    LOCK_INC(conn_fail_count);
    goto ERROR;
  }
  if (nread(fd, &i, sizeof(int)) == -1) {
    perror("connect");
    LOCK_INC(conn_fail_count);
    goto ERROR;
  }

  LOCK_INC(conn_success_count);

  for (i = 0; i < NUM_CACL; i++) {
    int a, b, c, ta, tb;
    a = i;
    b = i + 1;
    ta = htonl(a);
    tb = htonl(b);
    LOCK_INC(calc_count);
    if (nwrite(fd, &ta, sizeof(int)) == -1 || nwrite(fd, &tb, sizeof(int)) == -1) {
      perror("write");
      goto ERROR;
    }
    if (nread(fd, &c, sizeof(int)) == -1) {
      fprintf(stderr, "fd: %d ", fd);
      perror("read");
      LOCK_INC(calc_count);
      goto ERROR;
    }
    c = ntohl(c);
    if (c != a + b) {
      fprintf(stderr, "error: %d+%d=%d\n", a, b, c);
    } else {
      LOCK_INC(calc_success_count);
    }
  }

ERROR:
  close(fd);
  return NULL;
}

int main(int argc, char *argv[]) {
  int i, timeuse;
  void *ret;
  struct timeval start, end;

  server_ip = argv[1];

  gettimeofday(&start, NULL);

  if (argc > 2) total_client = atoi(argv[2]);

  for (i = 0; i < total_client; i++) {
    pthread_create(&threads[i], NULL, do_a_plus_b, NULL);
  }

  for (i = 0; i < total_client; i++) {
    pthread_join(threads[i], &ret);
  }

  gettimeofday(&end, NULL );
  timeuse = 1000 * ( end.tv_sec - start.tv_sec ) + (end.tv_usec - start.tv_usec) / 1000;

  printf("\nconnect successfully %d\n"
      "connect failed %d\n"
      "calculate %d\n"
      "calculate successfully %d\n"
      "total time %d.%ds\n",
      conn_success_count, conn_fail_count,
      calc_count, calc_success_count,
      timeuse / 1000, timeuse % 1000);
}
