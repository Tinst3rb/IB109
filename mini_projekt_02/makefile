CC = mpicc
CFLAGS = -std=c99 -Wall -Wextra -O2
TARGET = leader_election
SOURCE = main.c

all: $(TARGET)
	@mpirun -np 6 ./$(TARGET)

$(TARGET): $(SOURCE)
	@$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

clean:
	@rm -f $(TARGET)

.PHONY: all clean