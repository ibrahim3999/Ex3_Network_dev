# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c11

# Source files
SRCS_RECEIVER = TCP_Receiver.c
SRCS_SENDER = TCP_Sender.c

# Object files
OBJS_RECEIVER = $(SRCS_RECEIVER:.c=.o)
OBJS_SENDER = $(SRCS_SENDER:.c=.o)

# Executable names
EXEC_RECEIVER = TCP_Receiver
EXEC_SENDER = TCP_Sender

.PHONY: all clean

all: $(EXEC_RECEIVER) $(EXEC_SENDER)

$(EXEC_RECEIVER): $(OBJS_RECEIVER)
	$(CC) $(CFLAGS) -o $@ $^

$(EXEC_SENDER): $(OBJS_SENDER)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(EXEC_RECEIVER) $(EXEC_SENDER) $(OBJS_RECEIVER) $(OBJS_SENDER)
