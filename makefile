# Nome do compilador
CC = gcc

# Opções de compilação
CFLAGS = -Wall -Wextra -std=c99 -g

# Nome do executável
TARGET = program

# Arquivos de código-fonte
SRC = main.c connection.c

# Regra padrão
all: $(TARGET)

# Regra para criar o executável
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

connection.o: connection.c connection.h
	$(CC) $(CFLAGS) -c connection.c

# Limpar arquivos gerados
clean:
	rm -f $(TARGET)

.PHONY: all clean
