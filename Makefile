all: a_plus_b_basic

a_plus_b_basic: basic.c
	gcc -o a_plus_b_basic basic.c
