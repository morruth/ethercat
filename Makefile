CFLAGS=-c -g -Wall
BINARY=ethcat
LDFLAGS=-o $(BINARY)  -g
OBJS=ethcat.o ether.o logging.o config.o

.c.o:
	$(CC) $(CFLAGS) $< -o $@

all: $(BINARY)

$(BINARY): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS)
clean:
	rm -f ethcat *.o

