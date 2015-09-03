CFLAGS = -Wall -g -I../libserver/src 
LDFLAGS =  -L../libserver/build -lserver
SOURCES = ../src/main.c 
NAME = ../server

OBJECTS = $(SOURCES:.c=.o)

all: $(NAME)

$(NAME): lserver $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(NAME)

%.o: %.c
	$(CC) -c -fPIC  $(CFLAGS) $< -o $@

lserver:
	make -C ../libserver/build
clean:
	make -C ../libserver/build clean
	rm -f $(OBJECTS)
	rm -f $(NAME)