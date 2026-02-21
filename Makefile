build:
	clang -std=c23 -g -Wall -Wextra m2p.c -o m2p -l hpdf -l md4c
