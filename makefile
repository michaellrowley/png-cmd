OBJS	= main.o png_chunk.o utilities.o
SOURCE	= src/main.c src/png_chunk.c src/utilities.c
HEADER	= src/png-cmd.h
OUT	= png-cmd
CC	 = gcc
FLAGS	 = -g -c -w -Ofast
LFLAGS	 = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

main.o: src/main.c
	$(CC) $(FLAGS) src/main.c -std=c17

png_chunk.o: src/png_chunk.c
	$(CC) $(FLAGS) src/png_chunk.c -std=c17

utilities.o: src/utilities.c
	$(CC) $(FLAGS) src/utilities.c -std=c17

clean:
	rm -f $(OBJS) $(OUT)