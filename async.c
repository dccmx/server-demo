#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>


#include "common.h"

static int calc_success_count = 0;
static int calc_fail_count = 0;
static int accept_count = 0;

static void listener_cb(struct evconnlistener *, evutil_socket_t, struct sockaddr *, int socklen, void *);
static void conn_readcb(struct bufferevent *, void *);
static void conn_writecb(struct bufferevent *, void *);
static void conn_eventcb(struct bufferevent *, short, void *);
static void signal_cb(evutil_socket_t, short, void *);

int main(int argc, char **argv) {
  struct event_base *base;
  struct evconnlistener *listener;
  struct event *signal_event;

  struct sockaddr_in sin;

  base = event_base_new();
  if (!base) {
    fprintf(stderr, "Could not initialize libevent!\n");
    return 1;
  }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(PORT);

  listener = evconnlistener_new_bind(base, listener_cb, (void *)base,
      LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
      (struct sockaddr*)&sin,
      sizeof(sin));

  if (!listener) {
    fprintf(stderr, "Could not create a listener!\n");
    return 1;
  }

  signal_event = evsignal_new(base, SIGINT, signal_cb, (void *)base);

  if (!signal_event || event_add(signal_event, NULL)<0) {
    fprintf(stderr, "Could not create/add a signal event!\n");
    return 1;
  }

  printf("listenning on port %d...\n", PORT);

  event_base_dispatch(base);

  evconnlistener_free(listener);
  event_free(signal_event);
  event_base_free(base);

  printf("\naccept connection %d\n"
      "calculate successfully %d\n"
      "calculate failed %d\n",
      accept_count,
      calc_success_count, calc_fail_count);

  return 0;
}

static void listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data) {
  struct event_base *base = user_data;
  struct bufferevent *bev;

  bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
  if (!bev) {
    fprintf(stderr, "Error constructing bufferevent!");
    event_base_loopbreak(base);
    return;
  }
  accept_count++;
  bufferevent_setcb(bev, conn_readcb, conn_writecb, NULL, NULL);
  bufferevent_enable(bev, EV_READ|EV_WRITE);
  bufferevent_write(bev, &bev, sizeof(int));
}

static void conn_readcb(struct bufferevent *bev, void *user_data) {
  struct evbuffer *input = bufferevent_get_input(bev);
  size_t len = evbuffer_get_length(input);
  if (len >= 2 * sizeof(int)) {
    int a, b, c;
    bufferevent_read(bev, &a, sizeof(int));
    a = ntohl(a);
    bufferevent_read(bev, &b, sizeof(int));
    b = ntohl(b);
    c = htonl(a + b);
    bufferevent_write(bev, &c, sizeof(int));
    bufferevent_enable(bev, EV_WRITE);
    calc_success_count++;
  }
}

static void conn_writecb(struct bufferevent *bev, void *user_data) {
  struct evbuffer *output = bufferevent_get_output(bev);
  if (evbuffer_get_length(output) == 0) {
    bufferevent_disable(bev, EV_WRITE);
  }
}

static void conn_eventcb(struct bufferevent *bev, short events, void *user_data) {
  bufferevent_free(bev);
}

static void signal_cb(evutil_socket_t sig, short events, void *user_data) {
  struct event_base *base = user_data;
  event_base_loopexit(base, NULL);
}
