CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lpthread -lm

.PHONY: all clean experimentos

all: rotacionaMat geraMatriz

rotacionaMat: rotacionaMat.c
	$(CC) $(CFLAGS) -o rotacionaMat rotacionaMat.c $(LDFLAGS)
	@echo "Compilado: rotacionaMat"

geraMatriz: geraMatriz.c
	$(CC) $(CFLAGS) -o geraMatriz geraMatriz.c
	@echo "Compilado: geraMatriz"

# Gera a matriz e roda os 4 experimentos exigidos
experimentos: all
	./geraMatriz 1000 matriz1000.dat
	@echo "=== T=1 ===" && ./rotacionaMat 1000 1 matriz1000.dat matriz1000_t1.rot
	@echo "=== T=2 ===" && ./rotacionaMat 1000 2 matriz1000.dat matriz1000_t2.rot
	@echo "=== T=4 ===" && ./rotacionaMat 1000 4 matriz1000.dat matriz1000_t4.rot
	@echo "=== T=8 ===" && ./rotacionaMat 1000 8 matriz1000.dat matriz1000_t8.rot

clean:
	rm -f rotacionaMat geraMatriz *.dat *.rot *.o
