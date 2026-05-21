CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=c11 -Iinclude
SRC     = src/present.c src/hw_sim.c src/main.c
OBJ     = $(SRC:.c=.o)
TARGET  = present-sim

.PHONY: all run clean
all: $(TARGET)
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
run: all
	./$(TARGET)
clean:
	rm -f $(OBJ) $(TARGET)
