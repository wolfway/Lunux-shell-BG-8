OBJS=parser.o pos_3.o threads.o linked_list.o

CC=gcc
CFLAGS= -g -O -Wall -ansi

#Linux, Solaris
#PTHREAD_COMPILE=$(CC) $(CFLAGS) -lpthread  -lrt -o $@ $^

#FreeBSD
#PTHREAD_COMPILE=$(CC) $(CFLAGS) -pthread -o $@ $^

PTHREAD=if [ `uname` = 'SunOS' ]; then THREAD='-lpthread  -lrt';else THREAD='-pthread';fi
PTHREAD_LINK=$(PTHREAD);$(CC) $(CFLAGS) $$THREAD -o $@ $(OBJS)
PTHREAD_COMPILE=$(CC) $(CFLAGS) -c -o $@ $<

all: shell

shell: $(OBJS)
	$(PTHREAD_LINK)

.c.o:
	$(PTHREAD_COMPILE)

clean:
	rm -f *.o shell *.core

