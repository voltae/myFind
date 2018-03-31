# Define the required macros
CFLAGS = -Wall -Wextra -pedantic -std=gnu11
CC = gcc52
LDLIBS = -lm
OBJ = myFind.o

%.o: %.c
        $(CC) $(CFLAGS) -c $<

all: myFind

myFind: $(OBJ)
        $(CC) -o $@ $(OBJ) $(LDLIBS)

.phony : clean

clean:
        rm -f *.o myFind
