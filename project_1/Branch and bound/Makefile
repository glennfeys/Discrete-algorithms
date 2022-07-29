CC=g++
CFLAGS=-Wall -Wextra -std=c++2a -march=native -O3 -flto -I. -I../common # -I/home/niels/benchmark/include -L/home/niels/benchmark/build/src -lbenchmark -lpthread
HEADER_SOURCES=$(wildcard ../common/*.hpp)

.PHONY: all clean debug

all: executable

debug: CFLAGS += -fsanitize=undefined,address -g -Og
debug: executable

%.o: %.cpp $(HEADER_SOURCES)
	@$(CC) -c -o $@ $< $(CFLAGS)

executable: main.o ../common/graph.o
	$(CC) -o main $^ $(CFLAGS)

clean:
	rm -f ../common/*.o *.o main
