LDFLAGS =
CFLAGS=-g
SOURCES = src/server.c
NAME = server

OBJECTS = $(SOURCES:.c=.o)

all: $(NAME)


$(NAME): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(NAME)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS)
	rm -f $(NAME)
