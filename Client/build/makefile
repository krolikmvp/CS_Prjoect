CFLAGS=-g -Wall -I../libclient/src
LDFLAGS = -L../libclient/build -lclient
SOURCES = ../src/main.c
NAME = ../client

OBJECTS = $(SOURCES:.c=.o)

all: $(NAME)


$(NAME): lclient $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(NAME)

%.o: %.c
	$(CC) -c -fPIC $(CFLAGS) $< -o $@

lclient:
	make -C ../libclient/build
clean:
	make -C ../libclient/build clean
	rm -f $(OBJECTS)
	rm -f $(NAME)
