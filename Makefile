all: client a_plus_b_basic a_plus_b_mt a_plus_b_tp a_plus_b_async

client: client.c
	gcc -o client client.c -lpthread

a_plus_b_basic: basic.c
	gcc -o a_plus_b_basic basic.c

a_plus_b_mt: multi_thread.c
	gcc -o a_plus_b_mt multi_thread.c -lpthread

a_plus_b_tp: thread_pool.c
	gcc -o a_plus_b_tp thread_pool.c -lpthread

a_plus_b_async: async.c
	gcc -o a_plus_b_async async.c -levent
