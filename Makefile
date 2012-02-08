all: client a_plus_b_basic a_plus_b_mt a_plus_b_tp

client: client.c
	gcc -o client client.c

a_plus_b_basic: basic.c
	gcc -o a_plus_b_basic basic.c

a_plus_b_mt: multi_thread.c
	gcc -o a_plus_b_mt multi_thread.c

a_plus_b_tp: thread_pool.c
	gcc -o a_plus_b_tp thread_pool.c
