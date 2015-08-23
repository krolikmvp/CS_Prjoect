CFLAGS=-g
LDFLAGS =
SOURCES = src/client.c
NAME = client

OBJECTS = $(SOURCES:.c=.o)

all: $(NAME)


$(NAME): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(NAME)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS)
	rm -f $(NAME)
