CC = g++
CC_FLAGS = -g -std=gnu++17 -w -Wall -Wextra -pedantic-errors 

EXEC = altsa_server
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
LDFLAGS = -lpthread -lm -ldl -lstdc++fs -I./shared/ -I./shared/librg

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(EXEC)
 
%.o: %.cpp
	$(CC) $(LDFLAGS) -c $(CC_FLAGS) $< -o $@

clean:
	rm -f $(EXEC) $(OBJECTS)