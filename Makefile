CFLAGS=-c -g
BINARY=ethcat
LDFLAGS=-o $(BINARY) -g
OBJS=ethcat.o ether.o

.c.o:
	$(CC) $(CFLAGS) $< -o $@

all: $(BINARY)

$(BINARY): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS)
clean:
	rm -f ethcat *.o

