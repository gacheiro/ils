CC = g++
CFLAGS = -Wall -Wextra -std=c++14

OBJS = problem.o ils.o

TEST_SRC = src/*.h src/*.c

problem.o: src/problem.h src/problem.cpp
	$(CC) $(CFLAGS) -c src/problem.cpp

ils.o: src/ils.h src/ils.cpp
	$(CC) $(CFLAGS) -c src/ils.cpp

all: problem.o ils.o
	$(CC) $(CFLAGS) -o main src/main.cpp problem.o ils.o

clean:
	rm -f *.o
	rm -f a.out
	rm -f ils
	rm -f grasp
	rm -f test_all.out
