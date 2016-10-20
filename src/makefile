CC=g++
INC=-I./utils
CFLAGS=-c -Wall -std=c++0x -D NDEBUG -O3
LDFLAGS=-pthread
SOURCES=main.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=concurrent_linked_list

all:	$(SOURCES) $(EXECUTABLE)

$(EXECUTABLE):	$(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cc.o:
	$(CC) $(CFLAGS) $(INC) $< -o $@

clean:
	rm *.o concurrent_linked_list
