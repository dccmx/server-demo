#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

pthread_mutex_t init_count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int init_count = 0;
int total_client = 1000;
static pthread_t threads[1000];

void *do_a_plus_b(void *arg) {
  int fd, a, b, c, t, on = 1;
  struct sockaddr_in server_addr;

  pthread_mutex_lock(&init_count_mutex);
  init_count++;
  if (init_count == total_client) {
    fprintf(stderr, "\nGO!!!\n");
    pthread_cond_broadcast(&cond);
  } else {
    fprintf(stderr, "wait %4d ", init_count);
    if (init_count % 20 == 0) fprintf(stderr, "\n");
    pthread_cond_wait(&cond, &init_count_mutex);
  }
  pthread_mutex_unlock(&init_count_mutex);

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1 || setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
    perror("create socket");
    return NULL;
  }
  memset((void *) &server_addr, 0, sizeof(struct sockaddr_in));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_addr.sin_port = htons(1234);
  if (connect(fd, (const struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    perror("connect");
    return NULL;
  }
  a = rand() % 1000;
  b = rand() % 1000;
  t = htons(a);
  if (write(fd, &t, sizeof(int)) == -1) {
    perror("write");
  }
  t = htons(b);
  if (write(fd, &t, sizeof(int)) == -1) {
    perror("write");
  }
  if (read(fd, &c, sizeof(int)) == -1) {
    perror("write");
  }
  c = ntohs(c);
  if (c != a + b) {
    fprintf(stderr, "error: %d+%d=%d\n", a, b, c);
  }

  close(fd);
}

int main(int argc, char *argv[]) {
  int i;
  void *ret;
  srand(time(NULL) ^ getpid());
  if (argc > 1) total_client = atoi(argv[1]);
  for (i = 0; i < total_client; i++) {
    pthread_create(&threads[i], NULL, do_a_plus_b, NULL);
  }

  for (i = 0; i < total_client; i++) {
    pthread_join(threads[i], &ret);
  }
}
