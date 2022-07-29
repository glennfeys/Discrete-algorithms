CC=g++
CFLAGS=-Wall -Wextra -std=c++2a -march=native -O3 -DGRAPH_STARTS_AT_ZERO -flto -I. -I../common
HEADER_SOURCES=$(wildcard ../common/*.hpp)

.PHONY: all clean debug

all: executable

debug: CFLAGS += -g -Og # -fsanitize=undefined,address -g -Og
debug: executable

%.o: %.cpp $(HEADER_SOURCES)
	@$(CC) -c -o $@ $< $(CFLAGS)

graph.o: ../common/graph.cpp $(HEADER_SOURCES)
	@$(CC) -c -o $@ $< $(CFLAGS)

executable: main.o graph.o ../common/bitset.o
	$(CC) -o main $^ $(CFLAGS)

clean:
	rm -f ../common/*.o *.o main
