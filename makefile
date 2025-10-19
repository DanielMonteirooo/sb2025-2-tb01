CC = g++
CFLAGS = -std=c++17 -Wall -Werror

run: preprocessador montador

preprocessador: preprocessador.cpp
	$(CC) $(CFLAGS) -o preprocessador preprocessador.cpp
montador: montador.cpp
	$(CC) $(CFLAGS) -o montador montador.cpp
clean:
	rm -f preprocessador montador exemplo.pre exemplo.o1 exemplo.o2

.PHONY: run clean preprocessador montador
