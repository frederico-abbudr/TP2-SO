# Nome do compilador
CC = g++

# Flags do compilador
CFLAGS = -Wall -Wextra -O2

# Nome do arquivo executável
TARGET = simulador

# Lista de arquivos fonte
SRCS = simulator.c

# Lista de arquivos objeto (gerados a partir dos arquivos fonte)
OBJS = $(SRCS:.c=.o)

# Regra padrão para compilar o executável
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Regra para compilar os arquivos objeto
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para limpar os arquivos gerados
clean:
	rm -f $(TARGET) $(OBJS)

# Regra para rodar o programa
run: $(TARGET)
	./$(TARGET) $(ARGS)

.PHONY: clean run
