objects = reflector.o pty.o net.o misc.o
CC=gcc

reflector: $(objects)
	$(CC) -o reflector $(objects)

pty.o: pty.h
net.o: net.h
misc.o: misc.h
reflector.o: 

.PHONY: clean
clean:
	-rm reflector $(objects)
