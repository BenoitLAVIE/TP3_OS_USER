all : biceps

biceps : biceps.c server_thread.c creme.c creme.h
	cc -Wall -Werror -o biceps biceps.c server_thread.c creme.c -lreadline -lpthread

memory-leak :
	cc -Wall -Werror -g -O0 -o biceps-memory-leaks biceps.c server_thread.c creme.c -lreadline -lpthread

clean :
	rm -f biceps biceps-memory-leaks *.o