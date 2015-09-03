CFLAGS = -g -Wall
LDFLAGS = 
SOURCES =   ../src/server.c
NAME = libserver.so

OBJECTS = $(SOURCES:.c=.o)

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -shared -o $(NAME)

%.o: %.c
	$(CC) -fPIC -c  $(CFLAGS) $< -o $@

clean:

	rm -f $(OBJECTS)
	rm -f $(NAME)