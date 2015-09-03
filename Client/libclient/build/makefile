CFLAGS=-g -Wall
LDFLAGS =
SOURCES = ../src/client.c
NAME = libclient.so

OBJECTS = $(SOURCES:.c=.o)

all: $(NAME)


$(NAME): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -shared -o $(NAME)

%.o: %.c
	$(CC) -c -fPIC $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS)
	rm -f $(NAME)
