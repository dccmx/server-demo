all: client a_plus_b_basic

client: client.c
	gcc -o client client.c

a_plus_b_basic: basic.c
	gcc -o a_plus_b_basic basic.c
