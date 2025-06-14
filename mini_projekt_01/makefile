CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -pthread
TARGET = queue_test
SOURCES = main.c posix_queue.c lock_free_queue.c test.c

all: $(TARGET)
	@./$(TARGET)

$(TARGET): $(SOURCES)
	@$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	@rm -f $(TARGET)

.PHONY: all clean