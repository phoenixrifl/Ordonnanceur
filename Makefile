DEBUG	= yes

ifeq	($(DEBUG),yes)
	CFLAGS = -Wall -pthread 

else
	CFLAGS =
endif

CC	 = gcc

EXEC = lifo stealing

RM	= rm -rf

SRC	= quicksort.c \
	  schedLifo.c \
	  DequeTest.c \
	  sched.h

OBJS	= $(SRC:.c=.o)

all: all_normal clean

all_normal: $(EXEC)

lifo: quicksort.o schedLifo.o 
	$(CC) $(CFLAGS) -o $@ $^

stealing: quicksort.o dequeTest.o 
	$(CC) $(CFLAGS) -o $@ $^

dequeTest.o: dequeTest.c sched.h
	$(CC) $(CFLAGS) -c $<

quicksort.o: quicksort.c sched.h
	$(CC) $(CFLAGS) -c $<

schedLifo.o: schedLifo.c sched.h
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) *.o

mrproper: clean
	$(RM) $(EXEC)
