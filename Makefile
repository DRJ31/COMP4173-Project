CPP = g++
CFLAGS = -Wall -Wextra -pedantic
LIBS = $(shell pkg-config --cflags --libs opencv)
RM = rm -f
BIN = fusion
OBJS = $(patsubst %.cpp,%.o,$(wildcard *.cpp))

.PHONY: all clean

all: $(BIN)

clean:
	$(RM) $(BIN) $(OBJS)

$(BIN): $(OBJS)
	$(CPP) -o $@ $^ $(LIBS) $(CFLAGS)

%.o: %.cpp
	$(CPP) -g -o $@ -c $< $(CFLAGS)
