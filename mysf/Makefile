CC = gcc
CFLAGS = -O2 -pg -Wall -Iinclude

SRCS = src/btree.c src/access_ops.c src/acl_file.c src/acl_check.c src/acl_update.c src/fs_ops.c src/main.c src/scale_tests.c
OBJS = $(SRCS:.c=.o)

TARGET = myfs

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
